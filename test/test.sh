#!/usr/bin/env bash

TEST_URL='http://httpbin.org/ip'
CURL_ADDR_IPV4="curl -s -x socks5://localhost:23456"
CURL_ADDR_DOMAIN="curl -s -x socks5://localhost:23456"

expected=$(curl -s $TEST_URL)

SUCCESS_CONDS=(
    # noauth
    "$CURL_ADDR_IPV4 $TEST_URL"
    # username/password
    "$CURL_ADDR_IPV4 -U uusername:ppassword $TEST_URL"
    # proxy dns
    "$CURL_ADDR_DOMAIN $TEST_URL"
)

for cmd in "${SUCCESS_CONDS[@]}"; do
    real=$($cmd)
    if [ "$expected" == "$real" ]; then
        echo "passed:  " $cmd
    else
        echo "fail:    " $cmd
    fi
done
