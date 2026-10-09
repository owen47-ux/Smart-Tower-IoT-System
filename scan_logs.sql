-- phpMyAdmin SQL Dump
-- version 5.2.1deb1+deb12u1
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Aug 11, 2026 at 05:11 PM
-- Server version: 10.11.18-MariaDB-0+deb12u1
-- PHP Version: 8.2.32

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `security_db`
--

-- --------------------------------------------------------

--
-- Table structure for table `scan_logs`
--

CREATE TABLE `scan_logs` (
  `id` int(11) NOT NULL,
  `person_name` varchar(100) NOT NULL,
  `status` varchar(20) NOT NULL,
  `image_path` varchar(255) NOT NULL,
  `scanned_at` timestamp NULL DEFAULT current_timestamp()
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `scan_logs`
--

INSERT INTO `scan_logs` (`id`, `person_name`, `status`, `image_path`, `scanned_at`) VALUES
(1, 'No Face Detected', 'danger', '/home/owen/camera_captures/latest.jpg', '2026-08-11 15:54:25'),
(2, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 15:54:33'),
(3, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 15:54:42'),
(4, 'No Face Detected', 'danger', '/home/owen/camera_captures/latest.jpg', '2026-08-11 15:54:50'),
(5, 'No Face Detected', 'danger', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:04:35'),
(6, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:05:45'),
(7, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:15:55'),
(8, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:17:49'),
(9, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:17:57'),
(10, 'No Face Detected', 'danger', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:20:36'),
(11, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:21:09'),
(12, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:36:10'),
(13, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:36:19'),
(14, 'No Face Detected', 'danger', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:37:38'),
(15, 'No Face Detected', 'danger', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:37:51'),
(16, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:37:58'),
(17, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:42:20'),
(18, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:42:42'),
(19, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 16:45:33'),
(20, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 17:00:03'),
(21, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 17:02:50'),
(22, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 17:03:00'),
(23, 'owen', 'safe', '/home/owen/camera_captures/latest.jpg', '2026-08-11 17:03:06'),
(24, 'No Face Detected', 'danger', '/home/owen/camera_captures/latest.jpg', '2026-08-11 17:03:17');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `scan_logs`
--
ALTER TABLE `scan_logs`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `scan_logs`
--
ALTER TABLE `scan_logs`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=25;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
