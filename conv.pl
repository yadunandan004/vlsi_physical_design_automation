#!/bin/perl5 -w
#
# net2metis converter
#
# For unweighted cell areas
# Usage: net2metis XX.net > XX.hgr
#
# For weighted cell areas
# Usage: net2metis XX.net XX.are > XX.hgr
#

$file = $ARGV[0];
$op=$ARGV[1];
unless (open(INFILE, "$file")) {
    die("Can't open net file input file $file \n");
}
open my $fh, ">", "$op" or die "Cannot open output file";

#parsing netlist header
$line = <INFILE>;  # 0
$line = <INFILE>;  # nPins
$line = <INFILE>;  # nNets
chop($line);
@words = split(/[\t ]+/, $line);
$nNets = $words[0];
$line = <INFILE>;  # nNodes
chop($line);
@words = split(/[\t ]+/, $line);
$nNodes = $words[0];
$line = <INFILE>;  # padOffset
chop($line);
@words = split(/[\t ]+/, $line);
$padOffset = $words[0];
printf $fh "$nNets $nNodes ";
# if (@ARGV > 1) {
#     printf $fh "10";
# }


#parsing nets
while ($line = <INFILE>) {
    chop($line);
    @words = split(/[\tap ]+/, $line);
    if ($line =~ /s/) {
        print $fh "\n";
    }
    $num = $words[1];
    if ($line =~ /a/) {
        printf $fh "a"."%d ", $num;
    } else {
        printf $fh "p"."%d ", $num+$padOffset;
    }
}
printf $fh "\n";
close(INFILE);
close($fh)

# #parsing area file
# if (defined($ARGV[2])) {
#     $file = $ARGV[1];
#     unless (open(INFILE, "$file")) {
#         die("Can't open net file input file $file \n");
#     }

#     while ($line = <INFILE>) {
#         chop($line);
#         @words = split(/[\ ]+/, $line);
#         printf("$words[1]\n");
#     }
# }
