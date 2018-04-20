import requests, argparse, os.path

def upload(url, src_path, dst_path):
    print("upload %s,%s,%s" % (url,src_path,dst_path)) 
    fileName = 'list.htm'
    fileDataBinary = open(src_path, 'rb').read();
    files = { 'uploadFile': (dst_path, fileDataBinary, 'text/plain')}
    response = requests.post(url, files=files)
    return response.status_code == 200

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("url");
    parser.add_argument("files", metavar='file', type=str, nargs='+',
                        help='files to be upload')
    args = parser.parse_args()
    for file in args.files:
        upload(args.url,file,os.path.split(file)[1])
        

