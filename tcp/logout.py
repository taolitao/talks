import MySQLdb

def user_logout(*user, **sql_conf):
    conn = MySQLdb.connect(**sql_conf)
    cur = conn.cursor()
    update_info = "update user set ip='0.0.0.0' where userid='%s'" % user
    #print '////////////////update_info is', update_info
    res = cur.execute(update_info)
    if not res:
        print 'error logout'
    cur.close()
    conn.commit()
    conn.close()
    return update_info
