#include <iostream>

#include <restinio/all.hpp>
#include <json_dto/pub.hpp>

#include "weatherStation.hpp"


using weatherStation_t = std::vector< weatherRegistraiton_t >;

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

#include <string>
#include <restinio/websocket/websocket.hpp>

namespace rws = restinio::websocket::basic;

using ws_registry_t = std::map< std::uint64_t, rws::ws_handle_t >;

using traits_t = restinio::traits_t< restinio::asio_timer_manager_t, restinio::single_threaded_ostream_logger_t, router_t >;


class weatherStationHandler_t
{
public:
	explicit weatherStationHandler_t(weatherStation_t & registrations)
	: m_registrations{ registrations }
	{}
	
	auto onWeatherList(const restinio::request_handle_t& req, rr::route_params_t ) const
	{
		auto resp = init_resp( req->create_response() );

		resp.set_body(json_dto::to_json(m_registrations));

		return resp.done();
	}

	auto onWeatherPost(const restinio::request_handle_t& req, rr::route_params_t )
	{	
		auto resp = init_resp( req->create_response() );
		
		try
		{
			m_registrations.emplace_back(json_dto::from_json< weatherRegistraiton_t >( req->body() ) );
			sendMessage("POST: id = " + json_dto::from_json<weatherRegistraiton_t>(req->body()).m_id);
		}
		catch( const std::exception & /*ex*/ )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto onWeatherGetDate(const restinio::request_handle_t& req, rr::route_params_t params) const
	{
		auto resp = init_resp( req->create_response() );

		weatherStation_t dates;

		auto date = restinio::utils::unescape_percent_encoding( params[ "date" ] );

		for( auto &w :  m_registrations)
		{
			if(date == w.m_date)
			{
				dates.push_back(w);
			}
		}

		resp.set_body(json_dto::to_json(dates));

		return resp.done();
	}

	auto onGetThree(const restinio::request_handle_t& req, rr::route_params_t params) const
	{
		auto resp = init_resp( req->create_response() );

		weatherStation_t lastThree;

		for(std::size_t i = m_registrations.size() - 1; i > m_registrations.size() - 4; --i)
		{
			const auto& ref = m_registrations[ i ];
			lastThree.push_back(ref);
		}

		resp.set_body(json_dto::to_json(lastThree));

		return resp.done();
	}

	auto onUpdate( const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto id = restinio::cast_to< std::int64_t >( params[ "id" ] );

		auto resp = init_resp( req->create_response() );

		try
		{
			auto entry = json_dto::from_json< weatherRegistraiton_t >( req->body() );

			bool isID = false;
			for( auto &w :  m_registrations)
			{
				if(w.m_id == id)
				{
					w = entry;
					isID = true;
					sendMessage("PUT: id = " + json_dto::from_json<weatherRegistraiton_t>(req->body()).m_id);
				}
			}
			if(!isID)
			{
				mark_as_bad_request( resp );
				resp.set_body( "No weather data with #" + std::to_string( id ) + "\n" );
			}
		}
		catch( const std::exception & /*ex*/ )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto onDelete(const restinio::request_handle_t& req, rr::route_params_t params)
	{
		std::string path(req->header().path());
		const auto id = std::stoi(path.substr(path.find("delete/") + 7));

		auto resp = init_resp( req->create_response() );

		try
		{
			bool isID = false;
			for(auto it = m_registrations.begin(); it != m_registrations.end(); it++)
			{
				if(it->m_id == id)
				{
					m_registrations.erase(it--);
					isID = true;
					sendMessage("DELETE: id = " + id);
				}
			}
			if(!isID)
			{
				std::cout << "notID" << "\n";
				mark_as_bad_request( resp );
				resp.set_body( "No weather data with #" + std::to_string( id ) + "\n" );
			}
		}
		catch( const std::exception & ex )
		{
			std::cerr << ex.what();
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto options(restinio::request_handle_t req, restinio::router::route_params_t) // needs documentation
	{
		std::cout << "Options called " << "\n";
		const auto methods = "OPTIONS, GET, POST, PUT, DELETE, delete,";	
		auto resp = init_resp(req->create_response());
		resp.append_header(restinio::http_field::access_control_allow_methods, methods);
		resp.append_header(restinio::http_field::access_control_allow_headers, "content-type");
		resp.append_header(restinio::http_field::access_control_max_age, "86400");
		return resp.done();
	}

	auto on_live_update(const restinio::request_handle_t & req, rr::route_params_t params) 
	{
    	if (restinio::http_connection_header_t::upgrade == req->header().connection()) 
		{
	        auto wsh = rws::upgrade<traits_t>( *req, rws::activation_t::immediate, [this](auto wsh, auto m)
			{
                if (rws::opcode_t::text_frame == m->opcode() ||
					rws::opcode_t::binary_frame == m->opcode() ||
                    rws::opcode_t::continuation_frame == m->opcode()) 
					{
                    	wsh->send_message(*m);
                	} 
					else if (rws::opcode_t::ping_frame == m->opcode()) 
					{
                    	auto resp = *m;
                    	resp.set_opcode(rws::opcode_t::pong_frame);
                    	wsh->send_message(resp);
                	} 
					else if (rws::opcode_t::connection_close_frame == m->opcode()) 
					{
                    	m_registry.erase(wsh->connection_id());
                	}
            	});
        	m_registry.emplace(wsh->connection_id(), wsh);
       		init_resp(req->create_response()).done();
        	return restinio::request_accepted();
   		}
    	return restinio::request_rejected();
	}
	
private:
	weatherStation_t& m_registrations;

	ws_registry_t m_registry;
	
	void sendMessage(std::string message)
	{
		for(auto [k, v] : m_registry)
			v->send_message(rws::final_frame, rws::opcode_t::text_frame, message);
	}

	template < typename RESP >
	static RESP init_resp( RESP resp )
	{
		resp
			.append_header( "Server", "RESTinio sample server /v.0.6" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" )
			.append_header(restinio::http_field::access_control_allow_origin, "*");

		return resp;
	}

	template < typename RESP >
	static void mark_as_bad_request( RESP & resp )
	{
		resp.header().status_line( restinio::status_bad_request() );
	}
};


auto serverWeatherHandler(weatherStation_t & weatherStation)
{
	auto router = std::make_unique< router_t >();
	auto handler = std::make_shared< weatherStationHandler_t >( std::ref(weatherStation) );

	auto by = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, handler, _1, _2 );
	};

	// Handler for '/' path.
	router->http_get( "/", by( &weatherStationHandler_t::onWeatherList ) );
	// Handler for posting
	router->http_post( "/", by( &weatherStationHandler_t::onWeatherPost ) );
	// Handler for /date path
	router->http_get( "/date/:date", by( &weatherStationHandler_t::onWeatherGetDate) );
	// Handler for get last three post
	router->http_get( "/three/", by( &weatherStationHandler_t::onGetThree) );
	// Handler for update entry
	router->http_put( "/update-by-id/:id", by( &weatherStationHandler_t::onUpdate ) );
	router->add_handler(restinio::http_method_options(), "/update-by-id/:id", by(&weatherStationHandler_t::options));

	router->http_delete( "/delete/:id", by ( &weatherStationHandler_t::onDelete ) );
	router->add_handler(restinio::http_method_options(), "/delete/:id", by(&weatherStationHandler_t::options));

	router->add_handler(restinio::http_method_options(), "/", by(&weatherStationHandler_t::options));

	router->http_get("/chat", by(&weatherStationHandler_t::on_live_update));


	return router;
}

int main()
{
	using namespace std::chrono;

	try
	{

		weatherStation_t weatherStation{
			{1, "20211105", "12:15", {"Aarhus N", 13.692, 19.438}, 13.1, "70%"},
			{2, "20211105", "12:15", {"Aarhus N", 13.692, 19.438}, 13.1, "70%"},
			{3, "20211103", "12:14", {"Aarhus N", 13.692, 19.438}, 13.1, "60%"},
			{4, "20211103", "12:14", {"Aarhus S", 13.692, 19.438}, 13.1, "60%"},
			{5, "20211103", "12:14", {"Aarhus E", 13.692, 19.438}, 13.1, "60%"},
			{6, "20211103", "12:14", {"Aarhus V", 13.692, 19.438}, 13.1, "60%"},
			{7, "20211103", "12:14", {"Aarhus C", 13.692, 19.438}, 13.1, "60%"}
		};

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( serverWeatherHandler( weatherStation ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
