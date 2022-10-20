
import logging
 
LOG_FORMAT = "[%(asctime)s][%(levelname)s][%(module)s.py:%(lineno)d]---> %(message)s"
DATE_FORMAT = "%Y%m%d %T"
logging.basicConfig(filename="3rd/test_python/hello.log", level=logging.DEBUG, format=LOG_FORMAT, datefmt=DATE_FORMAT)

ch = logging.StreamHandler()    # 输出到控制台
ch.setLevel(logging.DEBUG)
ch.setFormatter(logging.Formatter(LOG_FORMAT,DATE_FORMAT))
logging.getLogger().addHandler(ch)


def func1(vender,dataid,tuple):
    logging.info("vender = %d, dataid = %d" % (vender,dataid))
    logging.info("type tuple = [%s] tuple = [%s]" % (str(type(tuple)), str(tuple)))
    return (15,5.6)

def hello(s):
    print(s)
    return (s)