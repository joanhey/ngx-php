
# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket "no_plan";

$ENV{'TEST_NGINX_BUILD_DIR'} = $ENV{'TRAVIS_BUILD_DIR'};

run_tests();

__DATA__
=== TEST 1: ini file
ini file

--- config
location = /ini {
    content_by_php '
        echo php_ini_loaded_file();
    ';
}
--- request
GET /ini
