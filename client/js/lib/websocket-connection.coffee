define (require, exports, module) ->
  Stream = require 'stream'
  WS = Stream.Writable
  RS = Stream.Readable
  EE = require 'events'
  Buffer = (require 'buffer').Buffer
  util = require 'util'
  
  module.exports = Connection = (url) ->
    if not this instanceof Connection
      return new Connection(url)
    self = this
    setTimeout ()->
      self._connect(url)
    , 0
    return this
  
  util.inherits Connection, EE
  
  Connection::_connect = (url)->
    try
      @rawSocket = new WebSocket url
      self = this
      @rawSocket.binaryType = 'arraybuffer'
      @rawSocket.onopen = ()->
        self.emit 'open'
      @rawSocket.onclose = (e)->
        self.emit 'close', 
          code: e.code
          reason: e.reason
          wasClean: e.wasClean
      @rawSocket.onerror = ()->
        self.emit 'error'
      @rawSocket.onmessage = (e)->
        data = e.data
        if typeof data == 'string'
          self.emit 'message', data 
          self.emit 'stringMessage', data
        else
          buf = new Buffer new Uint8Array data
          console.log buf;
          self.emit 'message', buf
          self.emit 'bufferMessage', buf
    catch err
      @emit 'error', err
  
  Connection::close = (code = 1000, reason)-> 
    try
      @rawSocket.close(code, reason)
    catch err
      @emit 'error', err
  
  Connection::sendString = (str)->
    try
      @rawSocket.send new String str
    catch err
      @emit 'error', err
  
  Connection::sendBuffer = (buf)->
    try
      @rawSocket.send buf.toArrayBuffer()
    catch err
      @emit 'error', err
  return Connection
  