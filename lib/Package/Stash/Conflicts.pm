package # hide from PAUSE
    Package::Stash::Conflicts;
use strict;
use warnings;

use Dist::CheckConflicts
    -dist => 'Package-Stash',
    -conflicts => {
        'Class::MOP'                    => '1.08',
        'MooseX::Role::WithOverloading' => '0.08',
        'namespace::clean'              => '0.18',
    };

1;

__END__
=pod

=head1 NAME

Package::Stash::Conflicts

=head1 VERSION

version 0.16

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2010 by Jesse Luehrs.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut

