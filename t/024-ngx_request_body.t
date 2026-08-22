# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: ngx_request_body
Test func ngx_request_body
--- config
location = /t1 {
    content_by_php_block {
        $body = ngx_request_body();
        echo $body."\n";
    }
}
--- request
POST /t1
Hello world
--- response_body
Hello world


=== TEST 2: ngx_request_body fragmented across multiple chunks
Request body should include all fragments in order
--- config
location = /t2 {
    content_by_php_block {
        $body = ngx_request_body();
        echo $body."\n";
    }
}
--- raw_request
POST /t2 HTTP/1.1
Host: localhost
Transfer-Encoding: chunked

5
Hello
5
 worl
5
d
0


--- response_body
Hello world
