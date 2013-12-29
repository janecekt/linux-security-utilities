#!/usr/bin/perl -w

# ====== Import and open syslog ======= #
use strict;

use Sys::Syslog;
openlog("blacklist[$$]", 'ndelay', 'user');



# ====== PARAMETERS ===== #
my($MAXTOKENS) = 40;
my($AGEOUT) = 24*60*60;
my($IPTABLES_1) = '/usr/sbin/iptables -A BLACKLIST -s ';
my($IPTABLES_2) = '/32 -j DROP';
my($line, $curTime, @ips, %tokens, %age ) = '';

sub LoginStart {
	my($action) = $_[0];
	my($service) = $_[1];
	my($user) = $_[2];
	my($ip) = $_[3];
	my($tokenIncrement) = $_[4];

	if ( !exists $tokens{$ip} ){
		$tokens{$ip} = 0;
		$age{$ip} = $curTime;
	}

	$tokens{$ip} += $tokenIncrement;
	
	if ( $tokens{$ip} >= $MAXTOKENS ){
		syslog('info',  $action . '(consumed ' . $tokenIncrement . ' tokens ' . ($MAXTOKENS - $tokens{$ip}) . ' left => IP BLACKLISTED) from ' . $ip . ' [' . $user . ' via ' . $service . ']' );
		system( $IPTABLES_1 . $ip . $IPTABLES_2 );
		delete  $tokens{$ip};
	} else {
		syslog('info',  $action . '(consumed ' . $tokenIncrement . ' tokens ' . ($MAXTOKENS - $tokens{$ip}) . ' left) from ' . $ip . ' [' . $user . ' via ' . $service . ']' );
	}
}

sub LoginSuccess {
	my($service) = $_[0];
	my($user) = $_[1];
	my($ip) = $_[2];

	if ( exists $tokens{$ip} ){
		delete $tokens{$ip};
		delete $age{$ip};
		syslog('info',  'Successful login from ' . $ip . ' [' . $user . ' via ' . $service . '] - deleted from limit !' );
	}
	else {
		syslog('info',  'Successful login from ' . $ip . ' [' . $user . ' via ' . $service . '] - no limit present - ignoring !' );
	}
}


while ( $line = <STDIN> ){
	chomp($line);

	# ===== Get Current Time ===== #
	$curTime = time();

	# ===== Cleanup aged enteries ===== #
	foreach( keys %age ){
		if ( $age{$_} + $AGEOUT < $curTime ){
			delete $tokens{$_};
			delete $age{$_};
			syslog('info',  $_ . ' aged out - deleted !' );
		}
	}

	# ===== Cut IP from string and skip if IP was not found === #
	syslog('debug', 'INPUT<' . $line . '>');

	if ( $line =~ m/.*SSH-SYN-PACKET.*SRC=(\d+\.\d+\.\d+\.\d+)/ ) {
		&LoginStart( 'Connect', 'sshd', '', $1 , 2);
		next;
	}

	if ( $line =~ m/.*FTP-SYN-PACKET.*SRC=(\d+\.\d+\.\d+\.\d+)/ ) {
		&LoginStart( 'Connect', 'vsftpd', '', $1, 2);
		next;
	}

	if ( $line =~ m/.*DROP-PACKET.*SRC=(\d+\.\d+\.\d+\.\d+).*DPT=(\d+)/ ) {
		&LoginStart( 'Forbidden access', 'port ' . $2, '', $1, 1 );
		next;
	}

	if ( $line =~ m/.*AUTH_AUDIT_DATA\[([^:]*):([^:]*):([^:]*):(\d+\.\d+\.\d+\.\d+)\]/ ) {
		## syslog('info', 'Line matches ' . $line);

		# === Accepted password - delete from log === #
		if ( $2 eq "setcred" ){
			&LoginSuccess($1,$3,$4);
			next;
		}

		# === Started logon form IP  === #
		if ( $2 eq "authenticate" ) {
			&LoginStart('Login', $1, $3, $4, 2);
			next;
		}

		syslog('info', 'Unsupported action ' . $2 . ' in ' . $line );
		next;
	}
	
	syslog('info', 'Unable to process ' . $line );
}


