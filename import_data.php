<?php
// ========== DATENBANK-KONFIGURATION ==========
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "satellite_db";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Verbindung fehlgeschlagen: " . $conn->connect_error);
}
echo "<h1>Daten-Import (Version mit Zeitstempel)</h1>";
echo "Datenbankverbindung erfolgreich.<br>";

$file_path = 'gpslog.jsonl';
if (!file_exists($file_path)) {
    die("Fehler: gpslog.jsonl nicht gefunden.");
}

$handle = fopen($file_path, "r");
if ($handle) {
    $stmt_log = $conn->prepare("INSERT INTO gps_logs (datetime, ms, fix, lat, lon, sats_in_use, sats_in_view, hdop, alt_m, speed_kmph) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    $stmt_sat = $conn->prepare("INSERT INTO satellite_readings (log_id, sat_id, elev, azim, snr) VALUES (?, ?, ?, ?, ?)");

    $line_count = 0;
    while (($line = fgets($handle)) !== false) {
        if (trim($line) === '') continue;
        $data = json_decode($line, true);
        if (json_last_error() !== JSON_ERROR_NONE || !isset($data['datetime'])) continue;
        
        $datetime = $data['datetime'];

       
        $stmt_log->bind_param(
            "siiddiiddd", 
            $datetime,
            $data['ms'],
            $data['fix'],
            $data['lat'],         // d
            $data['lon'],         // d
            $data['sats_in_use'], // i
            $data['sats_in_view'],// i
            $data['hdop'],        // d
            $data['alt_m'],       // d
            $data['speed_kmph']   // d
        );
        $stmt_log->execute();
        $last_log_id = $conn->insert_id;

        if (isset($data['satellites']) && is_array($data['satellites'])) {
            foreach ($data['satellites'] as $satellite) {
                $stmt_sat->bind_param("iiiii", $last_log_id, $satellite['id'], $satellite['elev'], $satellite['azim'], $satellite['snr']);
                $stmt_sat->execute();
            }
        }
        $line_count++;
    }
    fclose($handle);
    $stmt_log->close();
    $stmt_sat->close();
    echo "<h2>Import erfolgreich!</h2><p>" . $line_count . " Log-Einträge importiert.</p>";
}
$conn->close();
?>
