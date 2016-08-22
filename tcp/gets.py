import MySQLdb

def get_ip(*user, **sql_conf):
    conn = MySQLdb.connect(**sql_conf)
    cur = conn.cursor()
    info = "select ip from user where userid = '%s'" % user
    res = cur.execute(info)
    if not res:
        print 'no such user'
        ans = "error"
    else:
        ans = cur.fetchone()[0]
    cur.close()
    conn.commit()
    conn.close()
    return ans

#if __name__ == '__main__':
    #print get_ip("light", host="127.0.0.1", port=3306, user="root", passwd="", db="test")
