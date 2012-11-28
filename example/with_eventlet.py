import pydrizzle
import eventlet
from eventlet.hubs import trampoline

def wait_callback(fd, state):
    if state == pydrizzle.POLLIN:
        trampoline(fd, read=True)
    elif state == pydrizzle.POLLOUT:
        trampoline(fd, write=True)

pydrizzle.set_wait_callback(wait_callback)


def fetch(num):
    conn = pydrizzle.connect(user="root", port=3307, passwd="admin", db="test")
    cur = conn.cursor()
    cur.execute("select * from test")

pool = eventlet.GreenPool()
pool.spawn(fetch, 2)

pool.waitall()

