    #   T E S T C L . P L
    #   -----------------
    #   Test Client script to test the Win32::Pipe extension.
    #

    use Win32::Pipe;
    require "./test.pm";

    $TIMEOUT = 1;

        ####
        #   You may notice that named pipe names are case INsensitive!
        ####

    $PipeName = "\\\\$Server\\pipe\\$Pipe";

    foreach $Temp (@ARGV){
        $Temp =~ s/[\/|-]([a-z])/$1/ig;
        $Temp =~ tr/a-z/A-Z/;
        if ($Temp eq "T"){
            $Test = 1;
            print "Running in TEST mode...\n";
        }elsif ($Temp eq "D"){
            $Debug = 1;
            print "Running in DEBUG mode...\n";
        }
    }


    print "I am falling asleep for few seconds, so that we give time\nFor the server to get up and running.\n";
    sleep( $TIMEOUT ) unless $Test;
    print "\nOpening a pipe ...\n";


    print "Connecting to $PipeName\n";

    if ($Pipe = new Win32::Pipe($PipeName)){
        print "\n\nPipe has been opened, writing data to it...\n";
        print "-------------------------------------------\n";
        $bServerConnected = 1;

        $Pipe->Write( GetCredit( $Pipe ) );
        
        while( $bServerConnected )
        {
            print "\nCommands:\n";
            print "  FILE:xxxxx  Dumps the local file xxxxx to '$Server'.\n";
            print "  REMFILE:xxx Dumps the remote file xxx on '$Server'.\n";
            print "  RUN:xxxxx   Runs the command xxxxx on '$Server'.\n";
            print "  IMP         Force the server to impersonate your userid.\n";
            print "  UNIMP       Stop impersonating.\n";
            print "  Credit      Dumps the credit screen.\n";
            print "  Quit        Quits this client (server remains running).\n";
            print "  Exit        Exits both client and server.\n";
            print "  -----------------------------------------\n";
            print "  Enter Command or Message:\n";
            print "  >";

            $In = <STDIN>;
            chop($In);

            if( ( $File = $In ) =~ s/^file:(.*)/$1/i )
            {
                if( -s $File )
                {
                    if( open( FILE, "< $File" ) )
                    {
                        while( $File = <FILE> )
                        {
                            $In .= $File;
                        };
                        close( FILE );
                        undef $File;
                    }
                }
            }
            if( $In =~ /^credit$/i )
            {
                $In = GetCredit();
            }

            $iFlag2 = $Pipe->Write( $In );

            if($In =~ /^(exit|quit)$/i){
                print "\nATTENTION: Closing due to user request.\n";
                $bServerConnected = 0;
            }

            undef $In;

        }
        $Pipe->Close();
    }
    else
    {
        print "Error connecting: " . Win32::FormatMessage( $Win32::Pipe::Error ) . "\n";
        sleep(4);
    }

sub GetCredit
{
    my $Text, @Goo;

    @Goo = Win32::Pipe::Info();
    $Text = "\n\  " . join( "\n  ", @Goo ) . "\n\n";
    return( $Text );
}


