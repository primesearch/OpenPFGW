#!/usr/bin/perl -w
use strict qw(subs refs vars);
use Tk;
require Tk::ROText;

$|=1;

###########
# Constants
my $feini='pfgwfe.ini';


########
# Tweaks
my $compact=0;


###############
# Main Window
my $mainWindow=MainWindow->new;
$mainWindow->title("Primeform!");

my @modenames=('Expression Mode', 'File Mode');
my @modetags=('Expression', 'File Mode');
my $modename='unset';
my $mode=-1;
my $mode_configured=-1;

my @testdesc=('factor only',
              'probable primality test', 
              'Pocklington N-1 test',
              'Lehmer N+1 test',
              'Brillhart-Lehmer-Selfridge test',
              'decimal dump, no test');
my @testtags=('factor', 'PRP', 'N-1', 'N+1', 'BLS', 'dump');
my @testflags=('-o', '', '-t', '-tp', '-tc', '-od');
my $testtype=1;

my @factortags=('just test', 'factor first', 'factor deep');
my @factorflags=('', '-f', '-f -d');
my $factorstate=0;

my $state=0;
my $job='';
my $filename='';
my $linenumber='';
my $pid;

my $status='Welcome to LinPFGW!';


##########################
# Expresssion-mode globals
my $expression='';
my $exprregexp;
my @exprregexpvars=();
my @vars=();
my @letternames=('a','b','c','d','e','f');
my $maxmaxletter=5;
my $maxletter=-1;


###############
# Main menu
my $w_menubar = $mainWindow->Menu;
my $w_modemenu = $w_menubar->cascade(-label=>'~Mode',
                                     -tearoff=>0,
                                     );
for(0..$#modetags) 
{ 
    $w_modemenu->radiobutton(-label=>$modetags[$_], 
                             -variable=>\$mode, 
                             -value=>$_,
                             -command=>\&changeMode
                             ); 
}
my $w_testmenu = $w_menubar->cascade(-label=>'~Test',
                                     -tearoff=>0,
                                     );
for(0..$#testtags) 
{
    $w_testmenu->radiobutton(-label=>$testtags[$_], 
                             -variable=>\$testtype,
                             -value=>$_, 
                             -command=>\&changeTest); 
}
my $w_factormenu = $w_menubar->cascade(-label=>'~Factors',
                                       -tearoff=>0
                                       );
for(0..$#factortags) 
{
    $w_factormenu->radiobutton(-label=>$factortags[$_], 
                               -variable=>\$factorstate,
                               -value=>$_,
                               -command=>\&changeFactor); 
}

$mainWindow->configure(-menu=>$w_menubar);

$w_menubar->cascade(-label=>'~Help', 
                    -tearoff => 0, 
                    -menuitems => [ [Command=>'A~bout', -command => \&about] ]);
my $aboutReference;
sub about
{
    my $name = ref($aboutReference);
    if (defined($aboutReference))
    {
        $aboutReference->raise;
        $aboutReference->focus;
    }
    else
    {
        my $pop = $mainWindow->Toplevel();
        $pop->title("About");
	
        $pop->Label(text=>"LinPFGW")->pack();
        $pop->Label(text=>"Ver. 0.9b, 2004-04-16")->pack();
        $pop->Label(text=>"Copyright 2004")->pack();
        $pop->Label(text=>"Phil Carmody")->pack();
        $pop->Label(text=>"All Rights Reserved.")->pack();
        $pop->Label(text=>"You can distribute this freely and may modify")->pack();
        $pop->Label(text=>"it, so long as you make your modifications ")->pack();
        $pop->Label(text=>"freely available and retain this notice.")->pack();
        $pop->Label(text=>"Special Thanks to")->pack();
        $pop->Label(text=>"Chris Nash for the wonderful primeform")->pack();
        $pop->Label(text=>"Jim Fougeron for the maintaining pfgw")->pack();

        my $button_ok = $pop->Button(text=>'OK',
                                     command => sub {$pop->destroy();
                                                     $aboutReference = undef;
                                                 } )->pack();
        $pop->resizable('no','no');
        $aboutReference = $pop;
    }
}
sub changeTest
{
    if($testtags[$testtype] eq 'BLS')
    {
        $status = 'Alcohol Fuelled Primality!';
    }
    else
    {
        $status = "Test type changed to $testdesc[$testtype]";
        if($testtags[$testtype] eq 'factor' && !$factorstate) { $factorstate=1; }
    }
}
sub changeFactor
{
    if($testtags[$testtype] eq 'factor' && !$factorstate) 
    { 
        $factorstate=1; 
        $status="Factor mode is forced by virtue of the test mode";
    }
    else
    {
        $status = "Factor mode changed to $factortags[$factorstate]";
    }
}

###############
# Process/pipes
my $pfgwfh=undef;


################
# Feedback panel
my $w_feedbackframe = $mainWindow->Frame;
my $w_status=$w_feedbackframe->Label(-textvariable=>\$status);
$w_status->pack(-side=>'bottom');
my $w_feedbackbox=$w_feedbackframe->Scrolled('ROText', -setgrid=>'true', -height=>20, -scrollbars=>'e');
$w_feedbackbox->pack(-side=>'bottom');
my $w_feedback=$w_feedbackbox->Subwidget('rotext');

sub getpfgwline
{
    $_=<$pfgwfh>;
    if(!defined($_)) 
    { 
        # print STDERR "get got null\n"; 
        close($pfgwfh); $pfgwfh=undef; $/="\n";
        stopped();
        if($status !~ /Stopped/) { $status='Task completed'; }
    }
    else
    {
        # print $w_feedback, "\n";        
        s/\r/!CR!/g;
        s/\n/!LF!/g;
        #print "PFGW line : ``$_''\n -- end PFGW line\n";
        my @lines=split(/!LF!/, $_);
        {
            for my $line (@lines)
            {
                #print STDERR "PFGW_subline - ``$line''\n";
                if($line =~ s/!CR!//)
                {
                    $status=$line;
                }
                else
                {
                    if($line =~ m/Resuming .* at line (\d+)/)
                    {
                        $linenumber=$1;
                    }
                    elsif($line=~m/may have already been fully processed/)
                    {
                        $linenumber=1;
                    }
                    else
                    {
                        if(m/ (is (composite|PRP)|has (no small )?factor)/) 
                        { ++$linenumber; }

                        my @match=&regexpmatch($line);
                        # print STDERR ("[", join(':',@match), "]\n");
                        for my $i (0..$#match)
                        {
                            $vars[$exprregexpvars[$i]]->{'value'} = $match[$i];
                        }
                    }
                    $w_feedback->insert('end',"$line\n");
                }
            }
        }
        $w_feedback->yview('end');
    }
}

###############
# Main control
my $w_control = $mainWindow->Frame;
$w_control->Label(-textvariable=>\$modename)->pack(-side=>'left');

my $w_filename=$w_control->Entry(-textvariable=>\$filename);
$w_filename->pack(-side=>'right');
$w_control->Label(-text=>'File:')->pack(-side=>'right');
my $w_jobname=$w_control->Entry(-textvariable=>\$job);
$w_jobname->pack(-side=>'right');
$w_control->Label(-text=>'Job:')->pack(-side=>'right');

my $w_stop=$w_control->Button(-text=>'Stop', -command=>\&stop, -state=>'disabled');
$w_stop->pack(-side=>'right');
#$w_control->Button(-text=>'Pause', -command=>\&pause, -state=>'disabled')->pack(-side=>'right');
my $w_start=$w_control->Button(-text=>'Start', -command=>\&start);
$w_start->pack(-side=>'right');

sub stop
{
    if($state==0) { die("State=$state, and got 'stop'!"); }

    # Send signal to PFGW
    print STDERR "Handling stop with a big hammer\n";
    kill 'INT', $pid;
    $pid=undef;
    $status='Stopped by the user';
    # stopped();
}
sub stopped
{
#    close($pfgwfh); $pfgwfh=undef; $/="\n";

    $w_stop->configure(-state=>'disabled');

    if($mode==0) { expressionEnable(); }
    elsif($mode==1) { ; }
    chdir('..');
    # unlink($feini);
    $w_start->configure(-state=>'normal');
    $w_modemenu->configure(-state=>'normal');
    $w_testmenu->configure(-state=>'normal');
    $w_factormenu->configure(-state=>'normal');
    $w_jobname->configure(-state=>'normal');
    $state=0;
}
sub pause
{
    if($state==0) { die("State=$state, and got 'pause'!"); }
    # Send signal to PFGW
}
sub start
{
    if($state!=0) { die("State=$state, and got 'start'!"); }
    if(!$job) { $job="pfgw$$"; }
    if(-e $job && (!-d $job || !-w $job)) { die("Job name '$job' can't be used."); }
    if(!-d $job)
    {
        mkdir($job) or die("Can't create job directory '$job'.");
    }
    $w_start->configure(-state=>'disabled');
    $w_modemenu->configure(-state=>'disabled');
    $w_testmenu->configure(-state=>'disabled');
    $w_factormenu->configure(-state=>'disabled');
    $w_jobname->configure(-state=>'disabled');
    if($mode==0) 
    {
        expressionDisable(); 
        expressionCreateABC($job);
        expressionResetVars();
    }
    elsif($mode==1)
    {
        ;
    }
    
    if(open(FEINI, ">$feini"))
    {
        print FEINI "job=$job\n";
        print FEINI "filename=$filename\n";
        print FEINI "mode=$modetags[$mode]\n";
        print FEINI "test=$testtags[$testtype]\n";
        print FEINI "factor=$factortags[$factorstate]\n";
    }
    # Start pfgw
    chdir($job) or die("Can't chdir '$job'.");
 
    $w_stop->configure(-state=>'normal');
    $state=1;

    $/="\r";
    print STDERR "Command=pfgw $factorflags[$factorstate] $testflags[$testtype] $job.abc 2>&1|\n";
    $pid = open($pfgwfh, "pfgw $factorflags[$factorstate] $testflags[$testtype] $job.abc 2>&1|") or die("Can't open pfgw.");
    $w_feedback->fileevent($pfgwfh, 'readable', \&getpfgwline);
}




#######################
# EXPRESSION MODE FRAME
my $w_expressionframe=$mainWindow->Frame;


##################
# Expression panel
my $w_exprbar=$w_expressionframe->Frame;
$w_exprbar->Label(-text=>'Expression')->pack(-side=>'left');
#$w_exprbar->Button(-text=>'Start', -command=>\&exprStartStop);
my $w_expression=$w_exprbar->Entry(-textvariable => \$expression);
$w_expression->pack(-side=>'left', -expand=>1, -fill=>'x');


###################
# Variable rows
foreach my $var (0 .. $maxmaxletter)
{
    $vars[$var]={};
    my $ref=$vars[$var];
    $ref->{'value'}='???';

    for my $x qw/start end step/
    {
        $ref->{$x}='';
    }

    my $frame = $ref->{'widget'} = $w_expressionframe->Frame;
    $frame->Label(-text=>"$letternames[$var]:")->pack(-side=>'left');
    $frame->Label(-textvariable=>\$ref->{'value'})->pack(-side=>'left');

    my $typemenu=$ref->{'typewidget'} = 
        $frame->Menubutton(-text=>'Type', 
                           -underline=>0, 
                           -relief=>'raised',
                           -direction=>'below');
    $typemenu->command(-label=>'loop',
                       -command=>sub { setVariableType($var, 'loop'); } );
    $typemenu->command(-label=>'primes',
                       -command=>sub { setVariableType($var, 'primes'); } );
    $typemenu->pack(-side=>'right');
    $frame->Label(-textvariable=>\$ref->{'type'})->pack(-side=>'left');

    my $innerframe;

    # Loop widget
    $innerframe= $ref->{'loop'} = $frame->Frame;
    for my $x qw/start end step/
    {
        $innerframe->Label(-text=>"$x:")->pack(-side=>'left');
        $innerframe->Entry(-textvariable=>\$ref->{$x}, -exportselection=>1)->pack(-side=>'left');
    }

    # prime loop widget
    $innerframe = $ref->{'primes'} = $frame->Frame;
    for my $x qw/start end/
    {
        $innerframe->Label(-text=>"$x:")->pack(-side=>'left');
        $innerframe->Entry(-textvariable=>\$ref->{$x}, -exportselection=>1)->pack(-side => 'left');
    }
    $ref->{'type'}='loop';
    $ref->{'loop'}->pack(-side=>'left');
}
sub setVariableType
{
    my ($var,  $type)=@_;
    my $ref=$vars[$var];
    $ref->{$ref->{'type'}}->pack('forget');
    $ref->{'type'}=$type;
    $ref->{$type}->pack(-side=>'left');
}

########################
# Var configuration bar
my $w_varaddrembar=undef;
my ($w_varadd,$w_varrem);
if(!$compact) 
{
    $w_varaddrembar=$w_expressionframe->Frame; 
    $w_varadd=$w_varaddrembar->Button(-text=>'Add Var', -command=>\&addVar);
    $w_varadd->pack(-side=>'left');
    $w_varrem=$w_varaddrembar->Button(-text=>'Del Var', -command=>\&delVar);
    $w_varrem->pack(-side=>'left');
}
else
{
    $w_varadd=$w_exprbar->Button(-text=>'Add Var', -command=>\&addVar);;
    $w_varadd->pack(-side=>'left');
    $w_varrem=$w_exprbar->Button(-text=>'Del Var', -command=>\&delVar);
    $w_varrem->pack(-side=>'left');
}

sub addVar
{
    ${vars[++$maxletter]->{'widget'}}->pack(-side=>'top') if($maxletter < $maxmaxletter);
}
sub delVar
{
    ${vars[$maxletter--]->{'widget'}}->pack('forget') if($maxletter > 0) ;
}

#########################
# Build Expression window
$w_exprbar->pack(-side=>'top', -fill=>'x');
if(!$compact) { $w_varaddrembar->pack(-side=>'bottom'); }
addVar();


##################################
# Enable/Disable expression fields
sub expressionResetVars
{
    # $w_expressionframe->Busy(-recurse=>1);
    for my $var (0 .. $maxmaxletter)
    {
        $vars[$var]->{'value'}='???';
    }
}

sub expressionDisable
{
    # $w_expressionframe->Busy(-recurse=>1);
    $w_expression->configure(-state=>'disabled');
    for my $var (0 .. $maxmaxletter)
    {
        $vars[$var]->{'typewidget'}->configure(-state=>'disabled');
        my @varkids=($vars[$var]->{'loop'}->children,
                     $vars[$var]->{'primes'}->children);
        foreach(@varkids)
        {
            my $class=$_->class;
            if($class eq 'Entry' || $class eq 'Checkbutton')
            {
                $_->configure(-state=>'disabled');
            }
            # else { print "$var: " . $class . "\n"; }
        }
    }
    $w_varadd->configure(-state=>'disabled');
    $w_varrem->configure(-state=>'disabled');
}
sub expressionEnable
{
    # $w_expressionframe->Busy(-recurse=>1);
    $w_expression->configure(-state=>'normal');
    for my $var (0 .. $maxmaxletter)
    {
        $vars[$var]->{'typewidget'}->configure(-state=>'normal');
        my @varkids=($vars[$var]->{'loop'}->children,
                     $vars[$var]->{'primes'}->children);
        foreach(@varkids)
        {
            my $class=$_->class;
            if($class eq 'Entry' || $class eq 'Checkbutton')
            {
                $_->configure(-state=>'normal');
            }
            # else { print "$var: " . $class . "\n"; }
        }
    }
    $w_varadd->configure(-state=>'normal');
    $w_varrem->configure(-state=>'normal');
}
sub expressionCreateABC
{
    my $jobname=$_[0];
    chdir($jobname) or die("Can't chdir $jobname.");
    open(ABC, ">$jobname.abc") or die("Can't write $jobname/$jobname.abc.");
    my $rewriteexpr=$expression;
    if($rewriteexpr !~ m/\$[a-f]/)
    {
        $rewriteexpr =~ s/\b([a-f])\b/\$$1/g;
    }
    print ABC "ABC2 $rewriteexpr // autogenerated by linpfgw\n";
    for my $var (0..$maxletter)
    {
        my ($start,$end,$step,$primes);
        if(($start=$vars[$var]->{'start'}) eq '') { last; }
        if(($end=$vars[$var]->{'end'}) eq '') { last; }
        # print STDERR "$var is ", $vars[$var]->{'type'}, "\n";
        $primes=($vars[$var]->{'type'} eq 'primes') ? ' primes' : '';
        $step='';
        if(!$primes && $vars[$var]->{'step'}) 
        { 
            $step=" step ${vars[$var]->{'step'}}"; 
        }
        print ABC "$letternames[$var]:$primes from $start to $end$step\n";
    }
    close(ABC);
    regexpbuild();
    chdir('..');
}


#######################
# FILE MODE FRAME
my $w_fileframe=$mainWindow->Frame;
$w_fileframe->Label(-text=>'Expression')->pack(-side=>'left');
$w_fileframe->Entry(-textvariable=>\$expression, 
                    -state=>'disabled')->pack(-side=>'left', 
                                              -expand=>1,
                                              -fill=>'x');
$w_fileframe->Label(-text=>'Line Number')->pack(-side=>'left');
$w_fileframe->Label(-textvariable=>\$linenumber)->pack(-side=>'left');


#########################
# Mode changing
sub changeMode
{
    gotoMode($mode);
}
sub gotoMode
{
    if($state!=0) { die("State=$state, and got 'gotoMode $_[0]'!"); }
    if(!defined($modenames[$_[0]])) { die("got 'gotoMode $_[0]'?"); }
    if($mode_configured != $_[0])
    {
        leaveMode($mode_configured) if($mode_configured>=0);
        enterMode($_[0]);
    }
}
sub leaveMode
{
    if($mode_configured==0) { $w_expressionframe->pack('forget'); }
    elsif($mode_configured==1) { $w_fileframe->pack('forget'); }
    $mode_configured=undef;
    $modename=undef;
}
sub enterMode
{
    $mode=$mode_configured=$_[0];
    $modename=$modenames[$mode_configured];
    if($mode_configured==0) { $w_expressionframe->pack(-side=>'top'); }
    if($mode_configured==1) { $w_fileframe->pack(-side=>'top'); }
}

########
# Resume
sub resume
{
    if(open(INI, "<$_[0]"))
    {
        # print STDERR ("Trying to resume from $_[0]\n");
        
        while(my $line=<INI>)
        {
            chomp($line);
            if($line =~ m/^job\s*=\s*(\S+)/)
            {
                # print STDERR "Got $line > $job\n";
                $job=$1;
            }
            elsif($line =~ m/^file\s*=\s*(\S+)/)
            {
                # print STDERR "Got $line > $job\n";
                $filename=$1;
            }
            elsif($line =~ m/mode\s*=\s*(\w+\s*\w*)/)
            {
                for(0..$#modetags) { if($1 eq $modetags[$_]) { $mode = $_; last; } }
                if($mode != $mode_configured) { gotoMode($mode); }
                # print STDERR "$line: resume mode=$mode\n";
            }
            elsif($line =~ m/test\s*=\s*(\w+\s*\w*)/)
            {
                for(0..$#testtags) { if($1 eq $testtags[$_]) { $testtype = $_; last; } }
                # print STDERR "resume test=$testtype\n";
            }
            elsif($line =~ m/factor\s*=\s*(\w+\s*\w*)/)
            {
                for(0..$#factortags) { if($1 eq $factortags[$_]) { $factorstate = $_; last; } }
                # print STDERR "resume factor=$factorstate\n";
            }
        }
        close(INI);
    }
    # print STDERR ("Trying to resume from $_[0] => $job\n");
    return ($job && resumejob());
}
sub regexpbuild
{
    # print STDERR "Entering regexpbuild\n";
    @exprregexpvars=();
    my $rewriteexpr=$expression;
    if($rewriteexpr !~ m/\$[a-f]/) { $rewriteexpr =~ s/\b([a-f])\b/\$$1/g; }
    $rewriteexpr=~s/([\[\]\^\/()+*?])/\\$1/g;
    while($rewriteexpr =~ s/\$([abcdef])/(\\d+)/)
    {
        my $letter=$1;
        push @exprregexpvars, ord($letter)-ord'a';
    }
    # print STDERR "Generating regexp /$rewriteexpr/\n";
    $exprregexp=$rewriteexpr;
    my $sub = 
        'sub regexpmatch' .
        '{ ' .
        '  my @match=($_[0]=~m/'.$exprregexp.'/);' . 
        '  return @match; ' .
        '}';
    # print STDERR $sub, "\n";
    eval $sub;
}

sub resumejob
{
    if(!chdir($job)) { $job=''; return 0; }
    
    my $rv=1; # unless fail
    if(!$filename) { $filename="$job.abc"; }
    if(!open(ABC, "<$filename")) 
    { 
        $job=''; $filename='';
        chdir('..'); 
        return 0; 
    }
    my $abc=<ABC>;
    if($abc !~ m/^ABC(2)?\s+(.*\S)/) 
    {
        $job=''; $filename='';
        chdir('..'); 
        return 0; 
    }
    print STDERR ("ABC$1 $2\n");
    my $abc2=$1;
    $expression=$2;
    $expression=~s/\s*\/\/\s*(.*)//;
    my $lmaxvar=-1;
    if($abc2 eq '2')
    {
        while(<ABC>)
        {
            if(m/^([a-f])\s*:\s*(primes )?\s*from\s+(\d+)\s+to\s+(\d+)\s*(step\s+(\d+))?/)
            {
                my ($letter, $from, $to) = ($1, $3, $4);
                my $primes=$2?1:0;
                my $step=$5?$6:'';
                my $var=ord($letter)-ord('a');
                if($var!=$lmaxvar+1) 
                { print STDERR ("unexpected letter $letter\n"); } 
                if($var>$lmaxvar) { $lmaxvar=$var; }
                $vars[$var]->{'start'}=$from;
                $vars[$var]->{'end'}=$to;
                $vars[$var]->{'step'}=$step;
                setVariableType($var, $primes?'primes':'loop');
            }
        }
        while($maxletter<$lmaxvar) { addVar(); }
    }
    else
    {
    }
    close(ABC);
    regexpbuild();
    print STDERR "Resuming $job/$filename\nexpression=$expression\ncomment=$1\n";
    chdir('..');
    return $rv;
}

######################################
# Build main window, in expession mode
$w_control->pack(-side=>'top');
$w_feedbackframe->pack(-side=>'bottom');
if(!-r $feini || !resume($feini))
{
    gotoMode(0);
}
MainLoop;

