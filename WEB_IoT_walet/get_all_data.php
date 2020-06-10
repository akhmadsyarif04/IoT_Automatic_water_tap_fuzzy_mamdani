<?php
    include('koneksi.php');
    header('Content-Type: application/json');

$result_array = [];
$return_arr = array();

$sql = "SELECT * FROM data ORDER BY id DESC LIMIT 1";
// $sql = "SELECT * FROM data ORDER BY id DESC";
$result = $db->query($sql);
/* If there are results from database push to result array */
if ($result->num_rows > 0) {
    while($row = $result->fetch_assoc()) {

        $suhu = $row['suhu'];
        $kelembaban = $row['kelembaban'];
        $cahaya = $row['cahaya'];
        $date = $row['date'];
        $keran = $row['keran'];



        $return_arr[] = array("suhu" => $suhu,
        "kelembaban" => $kelembaban,
        "cahaya" => $cahaya,
        "date" => $date,
        "keran" => $keran
        );
    }
}

$json2 =  json_encode($return_arr);
echo $json2;


