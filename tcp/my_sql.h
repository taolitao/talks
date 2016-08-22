#ifndef _LEE_SQL_H
#define _LEE_SQL_H

void userLogin(char *addr, char *user, struct sys_conf *conf);

void userLogout(char *user, struct sys_conf *conf);

void getIp(char *ip, char *user, struct sys_conf *conf);

#endif
