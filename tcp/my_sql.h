#ifndef _LEE_SQL_H
#define _LEE_SQL_H

void userLogin(char *ip, unsigned short port, char *user, struct sys_conf *conf);

void userLogout(char *user, struct sys_conf *conf);

void getIpAndPort(char *res, char *user, struct sys_conf *conf);

#endif
