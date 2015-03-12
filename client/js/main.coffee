require.config
  baseUrl: 'js/lib',
  paths: 
    jquery: 'jquery-2.1.1.min'
define (require, exports, module) ->
  $ = require 'jquery'
  util = require 'util'
  window.bC = require 'cs!binaryChart'
  window.$ = $
  
  $().ready ()->
    b = new bC()
    window.b = b
    
    i = 10000
    state = 0;
    t = 0
    while (i > 0)
      b.addPlot((t = t + Math.floor(Math.random() * 100)), (state = !state))
      i -= 1;
    b.commit $ 'body'
    b._showTimeStart = 0
    b._showTimeEnd = 100
    b._refetch()
  
