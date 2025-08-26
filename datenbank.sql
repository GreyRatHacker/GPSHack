SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";

-- Eine der Tabellen speichert Informationen über den Standort, die Zeit und die allgemeine GPS-Leistung. 
-- Die andere speichert Details zu den einzelnen Satelliten, die in der ersten Tabelle erfasst wurden.

CREATE TABLE `gps_logs` (
  `id` int(11) NOT NULL,
  `datetime` datetime DEFAULT NULL,
  `ms` bigint(20) NOT NULL,
  `fix` tinyint(1) NOT NULL,
  `lat` decimal(10,8) NOT NULL,
  `lon` decimal(11,8) NOT NULL,
  `sats_in_use` int(11) NOT NULL,
  `sats_in_view` int(11) NOT NULL,
  `hdop` decimal(4,2) NOT NULL,
  `alt_m` decimal(10,2) NOT NULL,
  `speed_kmph` decimal(10,5) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;


CREATE TABLE `satellite_readings` (
  `id` int(11) NOT NULL,
  `log_id` int(11) NOT NULL,
  `sat_id` int(11) NOT NULL,
  `elev` int(11) NOT NULL,
  `azim` int(11) NOT NULL,
  `snr` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
