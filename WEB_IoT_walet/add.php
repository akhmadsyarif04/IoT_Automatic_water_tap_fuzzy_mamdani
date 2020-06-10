<?php
    include('koneksi.php');

    $suhu = $_GET['suhu'];
    $kelembaban = $_GET['kelembaban'];
    $cahaya = $_GET['cahaya'];
    $keran = $_GET['keran'];
    
    $name = 'syarif';
    // $suhu = 33.3;
    // $kelembaban = 33.3;
    // $cahaya = 44.2;
    // $keran = 44.3;

    // $data = '3232';
    // menginput data ke database
    $sql = "insert into data (name, suhu, kelembaban, cahaya, keran )" . 
    "values ('$name', '$suhu', '$kelembaban', '$cahaya','$keran')";
    $insert = mysqli_query($db, $sql);
    if ($insert) {
        echo "tersimpan";
    }else{
        echo "gagal tersimpan";
    }

?>