<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Irrigation System Dashboard</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f4f4f9;
            color: #333;
        }
        h1 {
            color: #2c3e50;
            text-align: center;
            margin-bottom: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .row {
            display: flex;
            flex-wrap: wrap;
            gap: 20px;
            margin-bottom: 20px;
        }
        .sensor-data, .photo-gallery {
            background: #fff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            flex: 1;
        }
        .sensor-data table {
            width: 100%;
            border-collapse: collapse;
        }
        .sensor-data th, .sensor-data td {
            padding: 10px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        .sensor-data th {
            background-color: #3498db;
            color: white;
        }
        .pump-status {
            padding: 5px 10px;
            border-radius: 5px;
            color: white;
            font-weight: bold;
        }
        .pump-status.on {
            background-color: #e74c3c; /* Red for ON */
        }
        .pump-status.off {
            background-color: #27ae60; /* Green for OFF */
        }
        .photo-gallery {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
        }
        .photo-gallery img {
            width: 100px;
            height: 100px;
            object-fit: cover;
            border-radius: 5px;
            cursor: pointer;
            transition: transform 0.2s;
        }
        .photo-gallery img:hover {
            transform: scale(1.1);
        }
        .modal {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0, 0, 0, 0.9);
            justify-content: center;
            align-items: center;
        }
        .modal img {
            max-width: 90%;
            max-height: 90%;
            border-radius: 8px;
        }
        .modal.active {
            display: flex;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Smart Irrigation System Dashboard</h1>

        <!-- Sensor Data and Photo Gallery Rows -->
        <?php
        // Database credentials
        $servername = "localhost";
        $username = "root"; // Default XAMPP username
        $password = ""; // Default XAMPP password
        $dbname = "smart_irrigation";

        // Create a database connection
        $conn = new mysqli($servername, $username, $password, $dbname);

        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        }

        // Fetch sensor data and photos
        $sensor_sql = "SELECT * FROM sensor_data ORDER BY timestamp DESC";
        $photo_sql = "SELECT * FROM photos ORDER BY upload_time DESC";

        $sensor_result = $conn->query($sensor_sql);
        $photo_result = $conn->query($photo_sql);

        // Combine sensor data and photos into rows
        $sensor_data = [];
        $photos = [];
        if ($sensor_result->num_rows > 0) {
            while ($row = $sensor_result->fetch_assoc()) {
                $sensor_data[] = $row;
            }
        }
        if ($photo_result->num_rows > 0) {
            while ($row = $photo_result->fetch_assoc()) {
                $photos[] = $row;
            }
        }

        // Display data in rows
        for ($i = 0; $i < max(count($sensor_data), count($photos)); $i++) {
            echo "<div class='row'>";
            
            // Sensor Data
            if ($i < count($sensor_data)) {
                $sensor = $sensor_data[$i];
                echo "<div class='sensor-data'>";
                echo "<h2>Sensor Data</h2>";
                echo "<table>";
                echo "<thead><tr><th>ID</th><th>Soil Moisture</th><th>Rain Sensor</th><th>Pump Status</th><th>Temperature</th><th>Humidity</th><th>Timestamp</th></tr></thead>";
                echo "<tbody>";
                echo "<tr>";
                echo "<td>" . $sensor['id'] . "</td>";
                echo "<td>" . $sensor['soil_moisture'] . "</td>";
                echo "<td>" . $sensor['rain_sensor'] . "</td>";
                echo "<td><span class='pump-status " . ($sensor['pump_status'] ? "on" : "off") . "'>" . ($sensor['pump_status'] ? "ON" : "OFF") . "</span></td>";
                echo "<td>" . $sensor['temperature'] . " °C</td>";
                echo "<td>" . $sensor['humidity'] . " %</td>";
                echo "<td>" . $sensor['timestamp'] . "</td>";
                echo "</tr>";
                echo "</tbody></table></div>";
            }

            // Photo Gallery
            if ($i < count($photos)) {
                $photo = $photos[$i];
                echo "<div class='photo-gallery'>";
                echo "<h2>Uploaded Photos</h2>";
                echo "<img src='" . $photo['file_path'] . "' alt='" . $photo['file_name'] . "' onclick='openModal(\"" . $photo['file_path'] . "\")'>";
                echo "</div>";
            }

            echo "</div>"; // Close row
        }

        // Close the connection
        $conn->close();
        ?>
    </div>

    <!-- Modal for Larger Image -->
    <div id="modal" class="modal" onclick="closeModal()">
        <img id="modal-img" src="" alt="Large Image">
    </div>

    <script>
        // Open modal with larger image
        function openModal(src) {
            const modal = document.getElementById('modal');
            const modalImg = document.getElementById('modal-img');
            modal.classList.add('active');
            modalImg.src = src;
        }

        // Close modal
        function closeModal() {
            const modal = document.getElementById('modal');
            modal.classList.remove('active');
        }
    </script>
</body>
</html>