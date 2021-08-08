**A collection of security utilities for Linux**

## Simple-Shell ##
A simple shell intended to use in conjugtion with OpenSSH ForceCommand or Chroot
  * the set working directory, umask
  * verify that environment variables are set (e.g. allow SFTP only)
  * exec predefined command (e.g. statically compiled SFTP-server)

## Pam-Trace ##
A simple PAM module which intercepts all PAM calls and sends them to SYSLOG in a user-defined format
  * Intercepts all PAM calls (based on configuration in pam.d of course)
  * Sends the PAM call name + PAM data (user, rhost, service, tty) to syslog in the user-defined format. These can be
    * Logged for audit purpouses
    * Consumed by IDS for further inspection e.g. to blacklist users/IPs in case of multiple unsuccessful login attempts.

## Blacklist ##
Simple token based IDS script which works in conjungtion with syslog-ng, iptables and pam-trace (above) to blacklist an IP trying to break into the system.
  * Each IP has X tokens to start with
  * A CONNECT attempt to an existing service consumes A tokens (reported via iptables -j LOG)
  * A CONNECT attempt to a closed port  consumes B tokens (reported via iptables -j LOG)
  * A LOGIN attempt to existing service consumes C tokens (reported via pam-trace above)
  * A SUCCESSFUL-LOGIN to existing sevice sets tokes to maximum again
  * If at any point a token count reaches 0 the IP is blacklisted (i.e. blocked via iptables).


# Contributions #
  * **If you wish to contribute and/or submit a pull request please do so on GitLab**
  * The repository is maintained on GitLab [https://gitlab.com/janecekt/linux-security-utilities](https://gitlab.com/janecekt/linux-security-utilities) 
  * It is mirrored to GitHub as a read-only copy but it is not maintained there any more
