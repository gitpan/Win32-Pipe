    #   T E S T S R . P L
    #   -----------------
    #   Test Server script to test the Win32::Pipe extension.
    #

    use Win32::Pipe;
    require "./test.pm";

    $PipeName = $Pipe;
    $NewSize = 2048;

    $| = 1;
    my $bServerContinue = 1;
    print "Creating pipe \"$PipeName\".\n";


# If you have Win32::Perms you could create a Win32::Perms object and
# specify it when creating the pipe...
    use Win32::Perms;
    $Perm = new Win32::Perms() || die "Unable to create a security descriptor.\n";
    $Perm->Allow( "roth.net\\administrators", FULL );
    $Perm->Deny( "roth.net\\guest" );
#    $Perm->Allow( "Marketing\\JOEL", FULL );
#    $Perm->Allow( "administrator", FULL );
#    $Perm->Owner( "Tech\\BILLY" );
#
# Now create the pipe and specify to use the permissions above
#    if( $Pipe = new Win32::Pipe( $PipeName, DEFAULT_WAIT_TIME, PIPE_READMODE_BYTE, $Perm ) )
#
 
    if( $Pipe = new Win32::Pipe( $PipeName, DEFAULT_WAIT_TIME, PIPE_READMODE_BYTE ) )
    {
        while( $bServerContinue )
        {
            $PipeSize = $Pipe->BufferSize();
            print "This pipe's current size is $PipeSize byte" . (($PipeSize == 1)? "":"s") . ".\nWe shall change it to $NewSize ...";
            print (($Pipe->ResizeBuffer($NewSize) == $NewSize)? "Successful":"Unsucessful") . "!\n\n";

            print "\n\nR e a d y   f o r   r e a d i n g :\n";
            print "-----------------------------------\n";

            print "Waiting for a client to connect...\n";

            if ($Pipe->Connect())
            {
                my $bClientConnected = 1;
                my $iMessage = 0;
                
                $User = ($Pipe->GetInfo()) [2];
                print "Pipe opened by $User.\n";

                 while( $bClientConnected )
                 {
                    my $In;
                    ++$iMessage;
                    print "Reading Message #$iMessage from $User: ";
                    $In = $Pipe->Read();

                    ####
                    #   In case the pipe is terminated on the client end
                    #   without telling us.
                    ####
                    if( ! $In )
                    {
                        $bClientConnected = 0;
                        print "Recieved no data, closing connection....\n";
                        next;
                    }

                    ####
                    #   In case the client wants to quit
                    ####
                    if( $In =~ /^quit/i )
                    {
                        print "\n\nQuitting this connection....\n";
                        $bClientConnected = 0;
                    }
                    elsif( $In =~ /^exit/i )
                    {

                        ####
                        #   In case the client wants to exit this entire test
                        ####
                        print "\n\nExitting.....\n";
                        $bClientConnected = $bServerContinue = 0;
                    }
                    elsif( $In =~ /^run:/i )
                    {
                        ####
                        #   If the a request to run a command is sent...
                        ####

                       ($Command) = ($In =~ /^run:(.*)/i);

                       print "Running a command:\n";
                       print "Command: '$Command'\n";
                       print "Output:\n", "-" x 40, "\n";

                       @Output = `$Command`;

                       map {print "$_";} @Output;

                       print "\n", "-" x 40, "\n";
                    }
                    elsif( ( $File = $In ) =~ s/^remfile:(.*)/$1/i )
                    {
                        print "\n  Dumping file: '$File':\n\n";
                        if( open( DATAFILE, "< $File" ) )
                        {
                            my( @Data ) = <DATAFILE>;
                            close( DATAFILE );
                            print join( "", @Data );
                        }
                        else
                        {
                            print "  Error: Unable to open file ($!)";
                        }
                        print "\n\n";
                    }
                    elsif( $In =~ /^imp/i )
                    {
                        print "\n  Impersonating '$User'...";
                        print ( ( $Pipe->ImpersonateClient() )? "Successful!" : "Failed" ) . "\n\n";
                    }
                    elsif( $In =~ /^unimp/i )
                    {
                        print "\n  Ending impersonation.\n\n";
                        $Pipe->StopImpersonation();
                    }
                    else
                    {
                        print "\"$In\"\n";
                    }
                }
                print "Disconnecting...\n";
                $Pipe->Disconnect();
            }
        }
        $Pipe->Close();
    }
    else
    {
        print "\nCould not create pipe\n";
        print "Error: " . Win32::FormatMessage( $Win32::Pipe::Error ) . "\n";
        print "Aborting...\n";
        exit;
    }



