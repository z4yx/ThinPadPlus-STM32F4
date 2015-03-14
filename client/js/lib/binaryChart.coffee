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
    @_canvas = undefined
    @_oriOptions = options
    @clear()
    this
  util.inherits BinaryChart, EE
  
  BinaryChart::clear = (options = @_oriOptions) ->
    @_showTimeStart = -1;
    @_showTimeEnd = 80;
    @strokeColor = options.strokeColor ? "#000000"
    @_points = []
    @_ended = false
    @_viewRect = options.viewPort ? {x: 0.1, y: 0.1, height: 0.8, width: 0.8}
    @_ratio = 2
    @_scale = 100
    @strokeSize = options.strokeSize ? 1
  
  BinaryChart::addPlot = (time, level) ->
    plot = new Point time, level
    @_showTimeStart = plot.time - 10 if @_points.lenght == 0
    @_showTimeEnd = plot.time + 10
    @_points.push plot
    self = this
    setTimeout ()->
      self._refetch()
    ,0
  
  BinaryChart::_findTime = (time) ->
    searchUtils.le @_points, time
  
  BinaryChart::commit = ($container) -> 
    @_canvas = ($('<canvas />').css
      width: '100%',
      height: '100%',
    .appendTo $container )[0]
    @_ended = true
    self = this
    ($ @_canvas).bind 'wheel', (e)->
      dx = e.originalEvent.deltaX * self._ratio
      dy = e.originalEvent.deltaY * self._ratio
      ox = e.originalEvent.offsetX * self._ratio
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
      if l >= 0.01
        self._showTimeStart = ot - l * left
        self._showTimeEnd = ot + l * (1 - left)
      
      self._refetch()
    $(window).resize (e)->
      self._refetch()
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
    cxt.fillStyle = @strokeColor
    cxt.lineWidth = @strokeSize * @_ratio
    cxt.strokeRect @_viewRect.x * @_width(), @_viewRect.y * @_height(), @_viewRect.width * @_width(), @_viewRect.height * @_height()
    @_drawSignal(cxt)
    @_drawScale(cxt)
    @emit 'viewDidRefetch'
    
  BinaryChart::_drawSignal = (cxt)->
    cxt.beginPath()
    nowPoint = @_findTime @_showTimeStart
    lastLevel = @_points[nowPoint]?.level
    lastLevel ?= true
    self = this
    getY = (level) ->
      (!level * self._viewRect.height + self._viewRect.y) * self._height() + (!!level * self._scale)
    cxt.moveTo @_viewRect.x * @_width(), getY(lastLevel)
    lastX = @_viewRect.x * @_width();
    rects = [];
    inRect = false
    while 1
      nowPoint += 1
      #console.log @_points[nowPoint]
      if not @_points[nowPoint]?
        didFinishDraw = true
        break
      if @_points[nowPoint].time > @_showTimeEnd
        didFinishDraw = false
        break
      nowX = @_timeToXPos(@_points[nowPoint].time)
      if nowX - lastX > 2
        if inRect
          inRect = false
          rects.push lastX
        cxt.moveTo lastX, getY(lastLevel)
        cxt.lineTo nowX, getY(lastLevel)
        lastLevel = @_points[nowPoint].level
        cxt.lineTo nowX, getY(lastLevel)
      else
        if not inRect
          rects.push lastX
          inRect = true
      lastX = nowX
      lastLevel = @_points[nowPoint].level
    if not didFinishDraw
      cxt.moveTo lastX, getY(lastLevel)
      cxt.lineTo (@_viewRect.x + @_viewRect.width) * @_width(), getY(lastLevel)
      
    cxt.stroke()
    i = 0
    while i < rects.length
      rectBegin = rects[i];
      rectEnd = rects[i + 1] ? lastX
      cxt.fillRect rectBegin, getY(1), rectEnd - rectBegin, getY(0) - getY(1)
      i += 2
    rects = undefined
  
  BinaryChart::_drawScale = (cxt) ->
    unit = Math.round (Math.log(100 / @_width() * (@_showTimeEnd - @_showTimeStart)) / Math.log(10))
    interval = 10 ** unit
    
    cxt.textAlign = 'center'
    cxt.font = @_ratio * 14 + 'px sans-serif'
    
    t = @_showTimeStart // interval * interval
    cxt.beginPath()
    count = t // interval
    while t <= @_showTimeEnd - interval
      t += interval
      count = (count + 1) % 10
      nextUnit = 0
      if count == 0
        nextUnit = 1
      cxt.moveTo @_timeToXPos(t), @_viewRect.y * @_height() + 0.95 * @_scale
      cxt.lineTo @_timeToXPos(t), @_viewRect.y * @_height() + (0.45 + 0.2 * !nextUnit) * @_scale 
      if nextUnit
        cxt.fillText beautify(t, unit + 1), @_timeToXPos(t), @_viewRect.y * @_height() + 0.3 * @_scale
    cxt.stroke()
  
  BinaryChart::_width = () ->
    $(@_canvas).width() * @_ratio
    
  BinaryChart::_height = () ->
    $(@_canvas).height() * @_ratio

  beautify = (number, unit) ->
    units = [
      'µs', 'ms', 's', 'Ks', 'Ms' 
    ]
    if unit < 0
      return number.toFixed(-unit) + units[0]
    else if unit > 12
      unit = 12
    unitLevel = Math.round(unit /3)
    leftDig = unitLevel * 3 - unit
    leftDig = 0 if leftDig < 0
    return (Math.round(number / (10 ** unit)) * (10 ** (unit - unitLevel * 3))).toFixed(leftDig) + units[unitLevel]
  BinaryChart
  
  
    
  