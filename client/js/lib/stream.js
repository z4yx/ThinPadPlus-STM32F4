define(function(require, exports, module) {
  var Stream = module.exports = require('_stream');
  Stream.Readable = require('_stream_readable');
  Stream.Writable = require('_stream_writable');
  Stream.Duplex = require('_stream_duplex');
  Stream.Transform = require('_stream_transform');
  Stream.PassThrough = require('_stream_passthrough');
});