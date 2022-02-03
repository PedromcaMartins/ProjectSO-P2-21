#!/bin/sh

for i in {1..3..1}; do
  ./client_server_simple_test /tmp/$i
done
