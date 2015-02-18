#ifndef HTTPFileWriting_H__
#define HTTPFileWriting_H__

#include "HTTPServer.h"
#include "ff.h"
#include <cstdlib>

class HTTPFileWritingHandlerData: public HTTPData {
public:
  FIL file;
  unsigned int remain;

  virtual ~HTTPFileWritingHandlerData() {
    f_close(&file);
  }
};

class HTTPFileWritingHandler : public HTTPHandler {
  public:
    /**
     * Create a new HTTPFileSzstemHandler.
     * @param prefix The Prefix is the URL Proefix in witch the Handler will work.
     * @param dir The Prefix will be directly mappt on the dir.
     */
    HTTPFileWritingHandler(const char *path, const char *dir) : HTTPHandler(path), _dir(dir) {}
    HTTPFileWritingHandler(HTTPServer *server, const char *path, const char *dir) : HTTPHandler(path), _dir(dir) { server->addHandler(this); }

  private:
    virtual void reg(HTTPServer *server) {
        server->registerField("Content-Length");
    }

    virtual HTTPStatus init(HTTPConnection *con) const {
      char filename[FILENAMELANGTH];
      HTTPFileWritingHandlerData *data = new HTTPFileWritingHandlerData();
      data->remain = atoi(con->getField("Content-Length"));

      snprintf(filename, FILENAMELANGTH, "%s%s\0", _dir, con->getURL() + strlen(_prefix));
      
      printf("write file: %s %d\n", filename, data->remain);
      
      if(FR_OK!=f_open(&data->file, filename, FA_WRITE|FA_CREATE_ALWAYS)) {
        delete data;
        return HTTP_NotFound;
      }
      
      con->data = data;
      con->setLength(0);
      return HTTP_OK;
    }

    /**
     * Send the maximum available data chunk to the Client.
     * If it is the last chunk close connection by returning HTTP_SuccessEnded
     * @param con The connection to handle
     * @param maximum The maximal available sendbuffer size.
     * @return HTTP_Success when mor data is available or HTTP_SuccessEnded when the file is complete.
     */
    virtual HTTPHandle send(HTTPConnection *con, int maximum) const {
      HTTPFileWritingHandlerData *data = static_cast<HTTPFileWritingHandlerData *>(con->data);
      printf("REST send()\r\n");

      if(data->remain == 0)
        return HTTP_SuccessEnded;
      else
        return HTTP_Success;
    }

    HTTPHandle data(HTTPConnection *con, void *buf, int len) const {
      HTTPFileWritingHandlerData *data = static_cast<HTTPFileWritingHandlerData *>(con->data);
      printf("REST data(%d)\r\n", len);
      // if(len == 0){
      //   data->finishedWriting = true;
      //   return HTTP_Success;
      // }
      data->remain -= len;
      unsigned int written;
      if(FR_OK!=f_write(&data->file, buf, len, &written)){
        printf("%s\n", "writing error");
      }
      return HTTP_Success;
    }

    
    /** The Directory which will replace the prefix of the URL */
    const char *_dir;
};

#endif

