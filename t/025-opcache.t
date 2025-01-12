
# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

$ENV{'TEST_NGINX_BUILD_DIR'} = $ENV{'TRAVIS_BUILD_DIR'};

run_tests();

__DATA__
=== TEST 1: opcache enabled
test opcache enabled
--- http_config
php_ini_path $TEST_NGINX_BUILD_DIR/.github/ngx-php/php/php.ini;
--- config
location = /opcache {
    content_by_php '
        echo opcache_get_status() === false ? "disabled" : "enabled\n";
    ';
}
--- request
GET /opcache
--- response_body
enabled



