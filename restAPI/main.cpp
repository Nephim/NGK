#include <iostream>

#include <restinio/all.hpp>
#include <json_dto/pub.hpp>

#include "weatherStation.hpp"


using weatherStation_t = std::vector< weatherRegistraiton_t >;

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

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
	

private:
	weatherStation_t& m_registrations;

	template < typename RESP >
	static RESP init_resp( RESP resp )
	{
		resp
			.append_header( "Server", "RESTinio sample server /v.0.6" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" );

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

	auto method_not_allowed = []( const auto & req, auto ) {
			return req->create_response( restinio::status_method_not_allowed() )
					.connection_close()
					.done();
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

		// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(), restinio::http_method_post() ),
			"/", method_not_allowed );

	return router;
}

int main()
{
	using namespace std::chrono;

	try
	{
		using traits_t =
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,
				router_t >;

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
