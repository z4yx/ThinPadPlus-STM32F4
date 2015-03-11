define (require, exports, module) ->
  $ = require 'jquery'
  util = require 'util'
  #EE is EventEmitter
  EE = (require 'events').EventEmitter
  searchUtils = require 'search-bounds'
  
  Point = (time, level) ->
    if not this instanceof Point
      return new Point(time, level)
      
    @time = time
    @level = level
  
  Point::valueOf = () ->
    @time
  
  module.exports = BinaryChart = (options = {})->
    if not this instanceof BinaryChart
      return new BinaryChart(options)
    EE.apply this 
    
    @_showTimeStart = 0;
    @_showTimeEnd = 1;
    @strokeColor = options.strokeColor ? "#000000"
    @_points = []
    @_ended = false
    @_canvas = undefined
    @_viewRect = options.viewPort ? {x: 0.1, y: 0.1, height: 0.8, width: 0.8}
    this
    
  util.inherits BinaryChart, EE
  
  BinaryChart::addPlot = (time, level) ->
    plot = new Point time, level
    throw new Error 'not-allowed'  if @_ended
    @_showTimeStart = plot.time if @_points.lenght == 0
    @_showTimeEnd = plot.time
    @_points.push plot
  
  BinaryChart::_findTime = (time) ->
    searchUtils.le @_points, time
  
  BinaryChart::commit = ($container) -> 
    @_canvas = ($('<canvas />').appendTo $container )[0]
    @_ended = true
    self = this
    ($ @_canvas).bind 'wheel', (e)->
      dx = e.originalEvent.deltaX
      dy = e.originalEvent.deltaY
      ox = e.originalEvent.offsetX
      if e.shiftKey 
        dx = dy
        dy = 0
      if Math.abs(dx) > Math.abs(dy)
        dy = 0
      else
        dx = 0
      dt = dx/(self._width() * self._viewRect.width)*(self._showTimeEnd - self._showTimeStart)
      dm = Math.exp(dy/(self._height() * self._viewRect.height))
      ot = self._xPosToTime ox
      
      self._showTimeStart += dt
      self._showTimeEnd += dt
      
      l = (self._showTimeEnd - self._showTimeStart)
      left = (ot - self._showTimeStart) / l
      l *= dm 
      self._showTimeStart = ot - l * left
      self._showTimeEnd = ot + l * (1 - left)
      
      self._refetch()
    $(window).resize (e)->
      self._refetch()
    
  BinaryChart::_timeToXPos = (time) ->
    ((time - @_showTimeStart)/(@_showTimeEnd - @_showTimeStart)*@_viewRect.width + @_viewRect.x) * @_width()
  BinaryChart::_xPosToTime = (xPos) ->
    (xPos/@_width() - @_viewRect.x)/@_viewRect.width * (@_showTimeEnd - @_showTimeStart) + @_showTimeStart
    
  BinaryChart::_refetch = () -> 
    @_canvas.width = @_width()
    @_canvas.height = @_height()
    cxt = @_canvas.getContext '2d'
    @emit 'viewWillRefetch'
    cxt.strokeStyle = @strokeColor
    cxt.beginPath()
    nowPoint = @_findTime @_showTimeStart
    lastLevel = @_points[nowPoint]?.level
    lastLevel ?= true
    self = this
    getY = (level) ->
      (!level * self._viewRect.height + self._viewRect.y) * self._height()
    cxt.moveTo @_viewRect.x * @_width(), getY(lastLevel)
    while 1
      nowPoint += 1
      #console.log @_points[nowPoint]
      if not @_points[nowPoint]?
        didFinishDraw = true
        break
      if @_points[nowPoint].time > @_showTimeEnd
        didFinishDraw = false
        break
      cxt.lineTo @_timeToXPos(@_points[nowPoint].time), getY(lastLevel)
      lastLevel = @_points[nowPoint].level
      cxt.lineTo @_timeToXPos(@_points[nowPoint].time), getY(lastLevel)
    cxt.lineTo (@_viewRect.x + @_viewRect.width) * @_width(), getY(lastLevel) if not didFinishDraw
    cxt.stroke()
    @emit 'viewDidRefetch'
    
  
  BinaryChart::_width = () ->
    $(@_canvas).width()
    
  BinaryChart::_height = () ->
    $(@_canvas).height()
  
  BinaryChart
  
  
    
  