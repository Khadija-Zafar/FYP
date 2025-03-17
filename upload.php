<?php
// Define the upload directory (relative path)
$upload_dir = "upload/"; // Relative to the root of your web server
$target_dir = __DIR__ . "/" . $upload_dir; // Absolute path for file operations

// Generate a unique filename using a timestamp
$timestamp = time(); // Get the current timestamp
$filename = "photo_" . $timestamp . ".jpg"; // Example: photo_1696523100.jpg
$target_file = $target_dir . $filename; // Absolute path for saving the file
$relative_file_path = $upload_dir . $filename; // Relative path for the database

// Initialize upload status
$uploadOk = 1;
$imageFileType = strtolower(pathinfo($target_file, PATHINFO_EXTENSION));

// Check if the file is an actual image
if (isset($_FILES["file"])) {
    $check = getimagesize($_FILES["file"]["tmp_name"]);
    if ($check !== false) {
        echo "File is an image - " . $check["mime"] . ".<br>";
        $uploadOk = 1;
    } else {
        echo "File is not an image.<br>";
        $uploadOk = 0;
    }
} else {
    echo "No file uploaded.<br>";
    $uploadOk = 0;
}

// Check file size (5MB limit)
if ($_FILES["file"]["size"] > 5000000) {
    echo "Sorry, your file is too large.<br>";
    $uploadOk = 0;
}

// Allow only certain file formats
if ($imageFileType != "jpg" && $imageFileType != "png" && $imageFileType != "jpeg" && $imageFileType != "gif") {
    echo "Sorry, only JPG, JPEG, PNG & GIF files are allowed.<br>";
    $uploadOk = 0;
}

// Check if $uploadOk is set to 0 by an error
if ($uploadOk == 0) {
    echo "Sorry, your file was not uploaded.<br>";
} else {
    // Attempt to move the uploaded file to the target directory
    if (move_uploaded_file($_FILES["file"]["tmp_name"], $target_file)) {
        echo "The file " . basename($_FILES["file"]["name"]) . " has been uploaded as " . $filename . ".<br>";

        // Insert into database
        // Database credentials
        $servername = "localhost";
        $username = "root"; 
        $password = ""; 
        $dbname = "smart_irrigation";

        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);

        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } else {
            echo "Connected to the database successfully.<br>";
        }

        // Escape special characters in filename and relative file path
        $filename = $conn->real_escape_string($filename);
        $relative_file_path = $conn->real_escape_string($relative_file_path);

        // Use prepared statements to prevent SQL injection
        $stmt = $conn->prepare("INSERT INTO photos (file_name, file_path, upload_time) VALUES (?, ?, NOW())");
        if ($stmt === false) {
            die("Prepare failed: " . $conn->error);
        }

        // Bind the filename and relative file path parameters
        $stmt->bind_param("ss", $filename, $relative_file_path);

        // Execute the statement
        if ($stmt->execute()) {
            echo "New record created successfully.<br>";
        } else {
            echo "Error: " . $stmt->error;
        }

        // Close the statement and connection
        $stmt->close();
        $conn->close();
    } else {
        echo "Sorry, there was an error uploading your file.<br>";
    }
}
?>