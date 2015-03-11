require.config({
  baseUrl: 'js/lib',
  paths:{
    'coffee-script': 'coffee-script'
  }
})
define(function(require, exports, module){
    exports = require('cs!../main.coffee');
});