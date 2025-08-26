<?php
// ========== DATENBANK-KONFIGURATION ==========

$servername = "localhost";
$username = "root";
$password = "";
$dbname = "satellite_db";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}


$sql = "SELECT * FROM gps_logs WHERE datetime IS NOT NULL ORDER BY datetime ASC";
$result = $conn->query($sql);

$output_data = [];
if ($result->num_rows > 0) {
    while($row = $result->fetch_assoc()) {
        $log_id = $row['id'];
        $sat_sql = "SELECT sat_id as id, elev, azim, snr FROM satellite_readings WHERE log_id = $log_id";
        $sat_result = $conn->query($sat_sql);
        $satellites = [];
        while($sat_row = $sat_result->fetch_assoc()) {
            $satellites[] = [
                'id' => (int)$sat_row['id'],
                'elev' => (int)$sat_row['elev'],
                'azim' => (int)$sat_row['azim'],
                'snr' => (int)$sat_row['snr']
            ];
        }
        $output_data[] = [
            'datetime' => $row['datetime'],
            'fix' => (bool)$row['fix'],
            'lat' => (float)$row['lat'],
            'lon' => (float)$row['lon'],
            'sats_in_use' => (int)$row['sats_in_use'],
            'sats_in_view' => (int)$row['sats_in_view'],
            'hdop' => (float)$row['hdop'],
            'alt_m' => (float)$row['alt_m'],
            'speed_kmph' => (float)$row['speed_kmph'],
            'ms' => (int)$row['ms'],
            'satellites' => $satellites
        ];
    }
}
$conn->close();

header('Content-Type: application/json');
echo json_encode($output_data);
?>
