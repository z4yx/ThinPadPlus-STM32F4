require.config
  baseUrl: 'js/lib',
  paths: 
    jquery: 'jquery-2.1.1.min'
define (require, exports, module) ->
  $ = require 'jquery'
  util = require 'util'
  bC = require 'cs!binaryChart'
  ws = require 'cs!websocket-connection'
  EE = require 'events'
  
  globalEvent = new EE()
  
  globalEvent.on 'connected', ()->
    $('#connect').prop 'disabled', true
    $('#start,#stop').prop 'disabled', false
    $('#error').text('')
  globalEvent.on 'started', ()->
    $('#start').prop 'disabled', true
    $('#stop').prop 'disabled', false
  .on 'stopping', ()->
    $('#stop').prop 'disabled', true
  .on 'stop', ()->
    $('#start').prop 'disabled', false
  .on 'error', (err)->
    $('#start,#stop').prop 'disabled', true
    $('#error').text(err)
    console.log err
    $('#connect').prop 'disabled', false
  
  $().ready ()->
    $('.tabButton').click ()->
      $('#main .tab').removeClass 'active'
    $('.tabButton').each (index)->
      $(this).click ()->
        $('#main .tab').eq(index).addClass 'active'
    logic = new bC
    logic.commit $('#logic')
    
    globalEvent.on 'logic', (data)->
      logic.addPlot(data.time, data.level)
    
    socket = {}
    $('#connect').click ()->
      socket = new ws($('#address').prop('value'))
      state = 0
      lastLevel = 0
      socket.on 'error', (e)->
        globalEvent.emit 'error', e
      .on 'open', ()->
        globalEvent.emit 'connected'
        socket.sendString('O' + $("#baud").prop('value')) 
      .on 'stringMessage', (e)->
        if e[0] == 'S'
          state = 0
        else if e[0] == 'A'
          state = 1
          globalEvent.emit 'logic', {time: 0, level: e[1] != '0'}
          lastLevel = e[1] != '0'
        else if e[0] == 'E'
          globalEvent.emit 'stop'
          globalEvent.emit 'logic', {time: parseInt e[1..], level: lastLevel}
      .on 'bufferMessage', (buf)->
        if state == 0 
          globalEvent.emit 'serial', buf
        else if state == 1
          number = 0
          leng = 0
          for rawnum in buf
            num = rawnum & 0x7f
            number += num << (leng * 7) 
            leng++
            if !(rawnum & 0x80)
              leng = 0;
              lastLevel = !lastLevel
              globalEvent.emit 'logic', {time: number, level: lastLevel}
              #number = 0
              # so number can be added continuously
    $('#start').click ()->
      socket.sendString('AB')
      bC(logic)
      globalEvent.emit 'started'
    $('#stop').click ()->
      socket.sendString('AE')
      globalEvent.emit 'stopping'     
