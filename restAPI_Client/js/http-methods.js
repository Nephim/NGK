function sendData()
{
	let entry = {
		id: parseInt(document.getElementById("id").value),
		date: document.getElementById("date").value,
		time: document.getElementById("time").value,
		place: {
			placeName: document.getElementById("placename").value,
			latitude: parseFloat(document.getElementById("latitude").value),
			longitude: parseFloat(document.getElementById("longitude").value)
		},
		temperature: parseFloat(document.getElementById("temperature").value),
		humidity: document.getElementById("humidity").value
	};

	console.log("Sent:");
	console.log(entry);
	axios.post('http://127.0.0.1:8080/', entry)
	.then(function (response) {
		console.log(response);
	})
	.catch(function (error) {
		console.log(error);
	});
}

function getData()
{
	axios.get('http://127.0.0.1:8080/')
	.then((response) => {
		console.log(response.data);
	}
	);
}
function getThree()
{
	axios.get('http://127.0.0.1:8080/three/')
	.then((response) => {
		console.log(response.data);
	}
	);
}