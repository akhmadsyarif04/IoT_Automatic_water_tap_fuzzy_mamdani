<?php
$server = "127.0.0.1";
$user = "root";
$password = "4";
$database = "sarang_walet_statisik_temperatur";
// $database = "arduino";

$db = mysqli_connect($server, $user, $password, $database);
if( !$db ){
die("Gagal terhubung dengan database: " . mysqli_connect_error());
}else{
    // echo "berhasil";
    // echo "<script>alert('asasas')</script>";
}
?>
