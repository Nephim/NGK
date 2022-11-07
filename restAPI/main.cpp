#include <iostream>

#include <restinio/all.hpp>

#include <json_dto/pub.hpp>

struct place_t
{
	place_t() = default;

	place_t(std::string placeName, float lat, float lon) : 
	m_placeName{ std::move( placeName ) },
	m_lat{ std::move( lat ) },
	m_lon{ std::move( lon ) }
	{}

	template < typename JSON_IO > void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "placeName", m_placeName )
			& json_dto::mandatory( "latitude", m_lat )
			& json_dto::mandatory( "longitude", m_lon );
	}

	std::string m_placeName;
	double m_lat, m_lon;
};

struct weatherRegistraiton_t
{
	weatherRegistraiton_t() = default;

	weatherRegistraiton_t(std::int64_t id, std::int64_t date, std::string time, place_t place, double temperature, double humidity) :
	m_id{ std::move( id ) },
	m_date{ std::move( date ) },
	m_time{ std::move( time ) },
	m_place{ std::move( place ) },
	m_temperature{ std::move( temperature ) },
	m_humidity{ std::move( humidity ) }
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "id", m_id )
			& json_dto::mandatory( "date", m_date )
			& json_dto::mandatory( "time", m_time )
			& json_dto::mandatory( "place", m_place )
			& json_dto::mandatory( "temperature", m_temperature )
			& json_dto::mandatory( "humidity", m_humidity );
	}

	std::int64_t m_id;
	std::int64_t m_date;
	std::string m_time;
	place_t m_place;
	double m_temperature;
	double m_humidity;
};

struct book_t
{
	book_t() = default;

	book_t(
		std::string author,
		std::string title )
		:	m_author{ std::move( author ) }
		,	m_title{ std::move( title ) }
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "author", m_author )
			& json_dto::mandatory( "title", m_title );
	}

	std::string m_author;
	std::string m_title;
};

using weatherStation_t = std::vector< weatherRegistraiton_t >;

using book_collection_t = std::vector< book_t >;

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

class weatherStationHandler_t
{
public:
	explicit weatherStationHandler_t(weatherStation_t & registrations) :
	m_registrations{ registrations }
	{}
	
	auto onWeatherList(const restinio::request_handle_t& req, rr::route_params_t ) const
	{
		auto resp = init_resp( req->create_response() );

		resp.set_body("Weather registrations:\n");

		for( std::size_t i = 0; i < m_registrations.size(); ++i )
		{
			const auto & w = m_registrations[ i ];
			resp.append_body(json_dto::to_json(w) + "\n");
		}

		return resp.done();
	}

private:
	weatherStation_t& m_registrations;

	template < typename RESP >
	static RESP
	init_resp( RESP resp )
	{
		resp
			.append_header( "Server", "RESTinio sample server /v.0.6" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" );

		return resp;
	}

	template < typename RESP >
	static void
	mark_as_bad_request( RESP & resp )
	{
		resp.header().status_line( restinio::status_bad_request() );
	}
};



class books_handler_t
{
public :
	explicit books_handler_t( book_collection_t & books )
		:	m_books( books )
	{}

	books_handler_t( const books_handler_t & ) = delete;
	books_handler_t( books_handler_t && ) = delete;

	auto on_books_list(
		const restinio::request_handle_t& req, rr::route_params_t ) const
	{
		auto resp = init_resp( req->create_response() );

		resp.set_body(
			"Book collection (book count: " +
				std::to_string( m_books.size() ) + ")\n" );

		for( std::size_t i = 0; i < m_books.size(); ++i )
		{
			resp.append_body( std::to_string( i + 1 ) + ". " );
			const auto & b = m_books[ i ];
			resp.append_body( b.m_title + "[" + b.m_author + "]\n" );
		}

		return resp.done();
	}

	auto on_book_get(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto booknum = restinio::cast_to< std::uint32_t >( params[ "booknum" ] );

		auto resp = init_resp( req->create_response() );

		if( 0 != booknum && booknum <= m_books.size() )
		{
			const auto & b = m_books[ booknum - 1 ];
			resp.set_body(
				"Book #" + std::to_string( booknum ) + " is: " +
					b.m_title + " [" + b.m_author + "]\n" );
		}
		else
		{
			resp.set_body(
				"No book with #" + std::to_string( booknum ) + "\n" );
		}

		return resp.done();
	}

	auto on_author_get(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		auto resp = init_resp( req->create_response() );
		try
		{
			auto author = restinio::utils::unescape_percent_encoding( params[ "author" ] );

			resp.set_body( "Books of " + author + ":\n" );

			for( std::size_t i = 0; i < m_books.size(); ++i )
			{
				const auto & b = m_books[ i ];
				if( author == b.m_author )
				{
					resp.append_body( std::to_string( i + 1 ) + ". " );
					resp.append_body( b.m_title + "[" + b.m_author + "]\n" );
				}
			}
		}
		catch( const std::exception & )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_new_book(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			m_books.emplace_back(
				json_dto::from_json< book_t >( req->body() ) );
		}
		catch( const std::exception & /*ex*/ )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_book_update(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto booknum = restinio::cast_to< std::uint32_t >( params[ "booknum" ] );

		auto resp = init_resp( req->create_response() );

		try
		{
			auto b = json_dto::from_json< book_t >( req->body() );

			if( 0 != booknum && booknum <= m_books.size() )
			{
				m_books[ booknum - 1 ] = b;
			}
			else
			{
				mark_as_bad_request( resp );
				resp.set_body( "No book with #" + std::to_string( booknum ) + "\n" );
			}
		}
		catch( const std::exception & /*ex*/ )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_book_delete(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto booknum = restinio::cast_to< std::uint32_t >( params[ "booknum" ] );

		auto resp = init_resp( req->create_response() );

		if( 0 != booknum && booknum <= m_books.size() )
		{
			const auto & b = m_books[ booknum - 1 ];
			resp.set_body(
				"Delete book #" + std::to_string( booknum ) + ": " +
					b.m_title + "[" + b.m_author + "]\n" );

			m_books.erase( m_books.begin() + ( booknum - 1 ) );
		}
		else
		{
			resp.set_body(
				"No book with #" + std::to_string( booknum ) + "\n" );
		}

		return resp.done();
	}

private :
	book_collection_t & m_books;

	template < typename RESP >
	static RESP
	init_resp( RESP resp )
	{
		resp
			.append_header( "Server", "RESTinio sample server /v.0.6" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" );

		return resp;
	}

	template < typename RESP >
	static void
	mark_as_bad_request( RESP & resp )
	{
		resp.header().status_line( restinio::status_bad_request() );
	}
};

auto weatherHandler(weatherStation_t & weatherStation)
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

	// Handlers for '/' path.
	router->http_get( "/", by( &weatherStationHandler_t::onWeatherList ) );

	// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
			restinio::http_method_get(), restinio::http_method_post() ),
			"/", method_not_allowed );

	return router;
}

auto server_handler( book_collection_t & book_collection )
{
	auto router = std::make_unique< router_t >();
	auto handler = std::make_shared< books_handler_t >( std::ref(book_collection) );

	auto by = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, handler, _1, _2 );
	};

	auto method_not_allowed = []( const auto & req, auto ) {
			return req->create_response( restinio::status_method_not_allowed() )
					.connection_close()
					.done();
		};

	// Handlers for '/' path.
	router->http_get( "/", by( &books_handler_t::on_books_list ) );
	router->http_post( "/", by( &books_handler_t::on_new_book ) );

	// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(), restinio::http_method_post() ),
			"/", method_not_allowed );

	// Handler for '/author/:author' path.
	router->http_get( "/author/:author", by( &books_handler_t::on_author_get ) );

	// Disable all other methods for '/author/:author'.
	router->add_handler(
			restinio::router::none_of_methods( restinio::http_method_get() ),
			"/author/:author", method_not_allowed );

	// Handlers for '/:booknum' path.
	router->http_get(
			R"(/:booknum(\d+))",
			by( &books_handler_t::on_book_get ) );
	router->http_put(
			R"(/:booknum(\d+))",
			by( &books_handler_t::on_book_update ) );
	router->http_delete(
			R"(/:booknum(\d+))",
			by( &books_handler_t::on_book_delete ) );

	// Disable all other methods for '/:booknum'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(),
					restinio::http_method_post(),
					restinio::http_method_delete() ),
			R"(/:booknum(\d+))", method_not_allowed );

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

		book_collection_t book_collection{
			{ "Agatha Christie", "Murder on the Orient Express" },
			{ "Agatha Christie", "Sleeping Murder" },
			{ "B. Stroustrup", "The C++ Programming Language" }
		};

		weatherStation_t weatherStation{
			{1, 20211105, "12:15", {"Aarhus N", 13.692, 19.438}, 13.1, 70},
			{2, 20211105, "12:15", {"Aarhus N", 13.692, 19.438}, 13.1, 70}
		};

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( weatherHandler( weatherStation ) )
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
