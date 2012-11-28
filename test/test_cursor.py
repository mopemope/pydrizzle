# -*- coding: utf-8 -*-

import pydrizzle

def test_execute():
    con = pydrizzle.connect(user="root", passwd="admin", db="test")
    assert(con != None)
    cursor = con.cursor()
    assert(cursor != None)
    cursor.execute("SELECT * FROM test")

def test_execute2():
    con = pydrizzle.connect(user="root", passwd="admin", db="test")
    assert(con != None)
    cursor = con.cursor()
    assert(cursor != None)
    cursor.execute("SELECT * FROM A WHERE A = ?", (1,))

test_execute()

