import MySQLdb

def user_login(*info, **sql_conf):
    conn = MySQLdb.connect(**sql_conf)
    cur = conn.cursor()
    update_info = "update user set ip = '%s' where userid = '%s'" % info
    res = cur.execute(update_info)
    if not res:
        print 'error update user info'
    cur.close()
    conn.commit()
    conn.close()
    return update_info
