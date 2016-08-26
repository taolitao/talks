import MySQLdb

def get_ip_port(*user, **sql_conf):
    conn = MySQLdb.connect(**sql_conf)
    cur = conn.cursor()
    info = "select ip, port from user where userid = '%s'" % user
    res = cur.execute(info)
    if not res:
        print 'no such user'
        ans = "error"
    else:
        ans = "%s:%d" % cur.fetchone()
    cur.close()
    conn.commit()
    conn.close()
    return ans

#if __name__ == '__main__':
    #print get_ip_port("light", host="127.0.0.1", port=3306, user="root", passwd="", db="test")
