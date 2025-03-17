<?php
// Set the response content type to JSON
header('Content-Type: application/json');

// Database credentials
$servername = "localhost";
$username = "root"; 
$password = ""; 
$dbname = "smart_irrigation";

// Create a database connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Connection failed: " . $conn->connect_error]));
}

// Read the raw POST data
$jsonData = file_get_contents('php://input');
$data = json_decode($jsonData, true);

// Check if data was received
if ($data) {
    // Extract data
    $soilMoisture = $data['soilMoisture'];
    $rainSensor = $data['rainSensor'];
    $pumpStatus = $data['pumpStatus'] ? 1 : 0; // Convert boolean to integer
    $temperature = $data['temperature'];
    $humidity = $data['humidity'];

    // Insert data into the database
    $sql = "INSERT INTO sensor_data (soil_moisture, rain_sensor, pump_status, temperature, humidity)
            VALUES ('$soilMoisture', '$rainSensor', '$pumpStatus', '$temperature', '$humidity')";

    if ($conn->query($sql) === TRUE) {
        echo json_encode(["status" => "success", "message" => "Data saved to database"]);
    } else {
        echo json_encode(["status" => "error", "message" => "Error: " . $sql . "<br>" . $conn->error]);
    }
} else {
    echo json_encode(["status" => "error", "message" => "No data received"]);
}

// Close the connection
$conn->close();
?>