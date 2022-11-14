#pragma once

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
	double m_lat;
	double m_lon;
};

struct weatherRegistraiton_t
{
	weatherRegistraiton_t() = default;

	weatherRegistraiton_t(std::int64_t id, std::string date, std::string time, place_t place, double temperature, std::string humidity) :
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
	std::string m_date;
	std::string m_time;
	place_t m_place;
	double m_temperature;
	std::string m_humidity;
};

