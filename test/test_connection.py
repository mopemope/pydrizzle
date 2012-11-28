# -*- coding: utf-8 -*-

import pydrizzle

def test_connect():
    con = pydrizzle.connect(port=3307, user="root", passwd="admin", db="test")
    assert(con != None)

def test_autocommit1():
    con = pydrizzle.connect(user="root", passwd="admin", db="test")
    assert(con != None)
    con.autocommit(True)

def test_autocommit2():
    con = pydrizzle.connect(user="root", passwd="admin", db="test")
    assert(con != None)
    con.autocommit(False)

def test_cursor():
    con = pydrizzle.connect(user="root", passwd="admin", db="test")
    assert(con != None)
    cursor = con.cursor()
    assert(cursor != None)

test_connect()

