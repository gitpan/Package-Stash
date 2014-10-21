#!/usr/bin/env perl
use strict;
use warnings;
use Test::More;
use Test::Fatal;

{
    package Foo;
    open *foo, "<", $0;

    sub foo { }
}

{
    package Bar;
    open *bar, "<", $0;

    sub bar { }
}

use Package::Stash;

{
    my $stash = Package::Stash->new('Foo');
    ok($stash->has_package_symbol('&foo'), "has &foo");
    ok($stash->has_package_symbol('foo'), "has foo");
    $stash->remove_package_symbol('&foo');
    ok(!$stash->has_package_symbol('&foo'), "has &foo");
    ok($stash->has_package_symbol('foo'), "has foo");
}

{
    my $stash = Package::Stash->new('Bar');
    ok($stash->has_package_symbol('&bar'), "has &bar");
    ok($stash->has_package_symbol('bar'), "has bar");
    $stash->remove_package_symbol('bar');
    ok($stash->has_package_symbol('&bar'), "has &bar");
    ok(!$stash->has_package_symbol('bar'), "has bar");
}

{
    my $stash = Package::Stash->new('Baz');
    ok(!exception {
        $stash->add_package_symbol('baz', *Foo::foo{IO});
    }, "can add an IO symbol");
    ok($stash->has_package_symbol('baz'), "has baz");
    is($stash->get_package_symbol('baz'), *Foo::foo{IO}, "got the right baz");
}

done_testing;
