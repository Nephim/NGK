#include "weatherStation.hpp"



weatherStationHandler_t::weatherStationHandler_t(weatherStation_t & registrations) 
	: m_registrations{ registrations }
	{}


restinio::response_builder_t<restinio::restinio_controlled_output_t> weatherStationHandler_t::onWeatherList(const restinio::request_handle_t& req, rr::route_params_t ) const
{
	auto resp = init_resp( req->create_response() );

	resp.set_body("[");

	for( std::size_t i = 0; i < m_registrations.size(); ++i )
	{
		const auto & w = m_registrations[ i ];
		resp.append_body(json_dto::to_json(w) + ",");
	}
	resp.append_body("]");

	return resp.done();
}


template < typename RESP >
RESP weatherStationHandler_t::init_resp( RESP resp )
{
	resp
		.append_header( "Server", "RESTinio sample server /v.0.6" )
		.append_header_date_field()
		.append_header( "Content-Type", "text/plain; charset=utf-8" );

	return resp;
}

template < typename RESP >
void weatherStationHandler_t::mark_as_bad_request( RESP & resp )
{
	resp.header().status_line( restinio::status_bad_request() );
}