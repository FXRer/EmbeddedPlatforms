use ExtUtils::MakeMaker;

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'         => 'Caraca',
    'VERSION_FROM' => 'Caraca.pm', # finds $VERSION
#    'linkext'   => {LINKTYPE => ''},
#    'dist'    =>    {COMPRESS=>'gzip', SUFFIX=>'gz'},
);
