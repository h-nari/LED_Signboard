<h1>メッセージ一覧</h1>

<div id="main">
  <form class="form-horizontal">

    <div class="form-group" class="part-block">
      <div class="col-xs-offset-7 col-xs-5">
        <button type="button" id="btn-new-msg1" class="btn btn-primary">
	  新規メッセージ</button>
        <button type="button" id="btn-write1" class="btn btn-primary">
	  書込</button>
        <button type="button" id="btn-renum1" class="btn btn-primary">
	  ID付け直し</button>
      </div>
    </div>

    <div id="msg-table" class="col-xs-11"></div>

    <div class="form-group" class="part-block">
      <div class="col-xs-offset-7 col-xs-5">
        <button type="button" id="btn-new-msg2" class="btn btn-primary">
	  新規メッセージ</button>
        <button type="button" id="btn-write2" class="btn btn-primary">
	  書込</button>
        <button type="button" id="btn-renum2" class="btn btn-primary">
	  ID付け直し</button>
      </div>
    </div>
    
    <div class="explain col-xs-offset-1 col-xs-11">
      <span class='ui-icon ui-icon-play'></span>
      をクリックするとメッセージをLEDに表示できます。
    </div>
    <div class="explain col-xs-offset-1 col-xs-11">
      <span class='ui-icon ui-icon-grip-solid-horizontal'></span>
      をドラグして、メッセージの順番を変えることができます。
    </div>
    <div class="explain col-xs-offset-1 col-xs-11">
      <span class='ui-icon ui-icon-pencil'></span>
      をクリックするとメッセージの内容を変更できます。
    </div>
  </form>
</div> <!-- main -->
<script src="/listPage.js">
</script>
