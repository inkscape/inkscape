#!/bin/bash
if ! test -e gtest ; then
    mkdir gtest
fi
(
    cd gtest
    if ! test -e gtest ; then
        wget -O 'googletest-release-1.7.0.tar.gz' https://github.com/google/googletest/archive/release-1.7.0.tar.gz
        tar -xvf 'googletest-release-1.7.0.tar.gz'
        mv googletest-release-1.7.0 gtest
    fi
    if ! test -e gmock-1.7.0 ; then
        wget -O 'googlemock-release-1.7.0.tar.gz' https://github.com/google/googlemock/archive/release-1.7.0.tar.gz
        tar -xvf 'googlemock-release-1.7.0.tar.gz'
        mv googlemock-release-1.7.0 gmock-1.7.0
    fi
)
