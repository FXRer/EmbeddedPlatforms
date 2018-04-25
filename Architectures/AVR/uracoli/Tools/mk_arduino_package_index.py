import json, hashlib, sys, shutil, os
from bottle import route, run, template, static_file

import bottle


try:
    import readline, rlcompleter
    readline.parse_and_bind("tab:complete")
except:
    pass



@route('/<filename:re:.*\.json>')
def send_json(filename):
    print "send_image:", filename
    return static_file(filename, root='/tmp', mimetype='application/json')

@route('/<filename:re:.*\.zip>')
def send_zip(filename):
    print "send_image:", filename
    return static_file(filename, root='/tmp', mimetype='application/zip')



if __name__ == "__main__":
    jf = open(sys.argv[1])
    jdata = json.load(jf)
    zf = sys.argv[2]
    zfd = open(zf).read()
    h = hashlib.new('sha256')
    h.update(zfd)
    chksum = h.hexdigest()
    print jdata
    p0 = jdata["packages"][0]['platforms'][0]

    #version = map(int, p0['version'].split("."))
    version = (0,5,0)
    p0['version'] = "%d.%d.%d" % tuple(version)
    p0['archiveFileName'] = os.path.basename(zf)
    p0['url'] = "http://localhost:8000/" + p0['archiveFileName']
    p0['checksum'] = 'SHA-256:%s' % chksum
    p0['size'] = len(zfd)

    fo = open("/tmp/package_uracoli_index.json","w")
    json.dump(jdata, fo, indent=4)
    fo.close()
    shutil.copyfile(zf, "/tmp/%s" % os.path.basename(zf))

    run(host='localhost', port=8000)
