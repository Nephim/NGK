function drawTable(){
	$("#table").tabulator({
		layout: "fitDataFill",
		height: "311px",
		columns: [{
			title: "ID",
			field: "id",
			align: "center"
		},
		{
			title: "Date",
			field: "date",
			align: "center"
		},
		{
			title: "Time",
			field: "time",
			align: "center"
		},
		{
			title: "Place Name",
			field: "place.placeName",
			align: "center"
		},
		{
			title: "Latitude",
			field: "place.latitude",
			align: "center"
		},
		{
			title: "Longitude",
			field: "place.longitude",
			align: "center"
		},
		{
			title: "Temperature",
			field: "temperature",
			align: "center"
		},
		{
			title: "Humidity",
			field: "humidity",
			align: "center"
		}
		],
	});  
}

function setTable(data)
{
	$("#table").tabulator("setData", data);
}
