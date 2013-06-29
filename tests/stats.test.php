<?php 

require_once 'testlib.php';

$g = new Gibson();

fail_if( $g->connect(GIBSON_SOCKET) == FALSE, "Could not connect to test instance" );
fail_if( is_array( $g->stats() )     == FALSE, "Unexpected STATS reply" );

$g->quit();

?>
