var msg_id;
var loaded_data;
var loaded_text = "";

$(function(){
    document.title = "メッセージ編集 / " + document.title;
    
    $("#overlay-select").change(select_overlay);
    $("#btn-write1,#btn-write2").on("click",writeMsg);
    $("#btn-disp1,#btn-disp2").on("click", function(){
        writeMsg();
        disp_msg(msg_id);
    });
    $("#btn-prev-msg").on("click", function(){ edit_next_msg(-1);}); 
    $("#btn-next-msg").on("click", function(){ edit_next_msg( 1);});
    $("#btn-title-set").on("click", titleSet);
    $("#btn-new-msg").on("click", new_msg);
    $("#btn-bitmap-file").on("change", bitmap_file);
    $("#bitmap-scrollType").on("change", onScrollTypeChange);
    $("#msg-bitmap").on("load", msg_bitmap_onload);
    select_overlay();
    var params = getParams();

    if(params) {
        msg_id = params.id[0];
        $("#msg-id").val(msg_id);
        load_msg_file();
        onScrollTypeChange();
    }

    $(window).on('beforeunload',function(e){
        if(modified()){
            return "ちょっと待ってください。"
        }
    });
    load_font_list();
});

function modified() {
    var d = get_msg_data();
    var cnt = 0;
    for(let k in loaded_data){
        if(loaded_data[k] != d[k]){
            console.log(k + " changed");
            cnt++;
        } 
    }

    if(d.player == "scroll-text"){
        var text = $("#scroll-text-text").val();
        if(loaded_text != text){
            console.log("text changed");
            cnt++;
        }
    }
    return cnt > 0;
}

function getParams(){
    "use strict";
    var query = location.search.substr(1);
    if(query.length < 1) return null;
    var params = {};
    query.split("&").forEach(function(item){
        var s = item.split("=");
        var k = decodeURIComponent(s[0]);
        var v = decodeURIComponent(s[1]);
        if(k in params) params[k].push(v);
        else params[k] = [v];
    });
    return params;
}

function load_msg_file()
{
    status("load_msg_file");
    read_msg_file(msg_id, {
        success: function(data, dataType){
            loaded_data = data;
            set_msg_data(data);
            load_font_list( data.font );
            if(data.player == "scroll-text"){
                load_txt_file();
            }
            if(data.player == "bitmap"){
                // var s = "<img src='/api?cmd=read_bmp&id=";
                // s += msg_id + "'>";
                // $("#canvas-container").html(s);
                var img = $("#msg-bitmap")[0];
                img.src = "/api?cmd=read_bmp&id=" + msg_id;
            }
            status("load_msg_file successed");
        }
    });
}

function set_msg_data(data)
{
    var player = data.player;
    $("#msg-title").val(data.title || "");
    $("#overlay-select").val(player);
    select_overlay();

    for(var prop in data){
        if(prop == "title" || prop == "player") continue;
        let name1 = "#msg-" + prop;
        let name2 = "#" + player + "-" + prop;
        let name = name1 + "," + name2;
        let type = $(name).prop("type");

        if(type == "checkbox"){
            $(name).prop("checked", data[prop]);
        } else {
            $(name).val(data[prop]);
        } 
    }
}        


function load_txt_file()
{
    status("load_txt_file");
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "text",
        data: {cmd : "read_txt", id: msg_id},
        success : function(data, dataType){
            status("load_txt_file succeeded");
            var nl = data.match(/\n/g);
            var lc = nl ? nl.length : 0;
            var rows = Math.min(20,lc);

            loaded_text = data;
            $("#scroll-text-text").val(data);
            $("#scroll-text-text").attr("rows", rows);
        },
        error: function(XMLHttpRequest, textStatus, e){
            status("no htm file for id="+ msg_id + " :"
                   + textStatus + ":" + e.message);
        },
    });
}


function select_overlay(){
    var opt = $("#overlay-select option:selected");
    var target = opt.attr("target");
    var parts = $("#editor-overlay-part > div");
    for(let page of parts){
        if(page.id == target) $(page).show();
        else $(page).hide();
    }
}

function get_msg_data()
{
    var id = $("#msg-id").val();
    var player = $("#overlay-select").val();
    var d = { player: player};
    
    for(let n of ['title','rx','ry','rw','rh','bg','pre-clear',
                  'post-clear']){
        let name = "#msg-" + n;
        let type = $(name).prop("type");
        if(type == "checkbox") {
            d[n] = $(name).prop("checked");
        } else {
            d[n] = $(name).val();
        } 
    }

    var part = "#"+player+"-player-part";
    var items = $(part + " input," +
                  part + " select");
    var prefix_len = player.length + 1;
    for(let item of items){
        if(item.id.startsWith(player + "-")){
            let name = item.id.substring(prefix_len);
            if(item.tagName == "INPUT" && $(item).prop("type") == "checkbox")
                d[name] = $(item).prop("checked");
            else
                d[name] = $(item).val();
        }
    }
    return d;
}
    
function writeMsg()
{
    var d = get_msg_data();
    loaded_data = d;
    
    upload_file(msg_id, 'msg', d);
    if( d.player == "scroll-text"){
        loaded_text = $("#scroll-text-text").val();
        upload_file(msg_id, 'txt', loaded_text);
    }
    else if( d.player == "bitmap"){
        upload_bmp();
    }

    update_index(msg_id, d.player, $("#msg-title").val());
}

function update_index(id, player, title){
    $.ajax({
        type: "get",
        url: "/api" ,
        dataType: "json",
        data: {cmd: "read_index"},
        success: function(data, dataType){
            var msg = data.msg;
            for(let i=0; i<msg.length;i++){
                if(msg[i].id == id){
                    data.msg[i].player = player;
                    data.msg[i].title  = title;
                    write_index(data, false);
                    return;
                }
            }
            console.log("id:" + id + " not found in index file");
        },
        error: function(XMLHttpRequest, textStatus, e){
            console.log("fail to read index:" + textStatus + e.message);
            return;
        }
    });
}

function load_font_list(font = 0)
{
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "json",
        data: {cmd: "font_list"},
        success: function(data, dataType){
            var s = "";
            for(let i=0; i<data.length; i++){
                s += "<option value='";
                s += i;
                s += "'";
                if(i == font)
                    s += " selected";
                s += ">";
                s += data[i];
                s += "</option>";
            }
            $("#static-text-font,#scroll-text-font").html(s);
        },
        error: function(XMLHttpRequest, textStatus, e){
            console.log("fail to get font_list:" + textStatus + e.message);
            return;
        }
    });
}

function edit_next_msg( delta )
{
    if(modified()){
        if(!confirm("データが変更されています。\n" +
                    "書込後、移動しますか？"))
            return;
        writeMsg();
    }
    
    load_index({success: function(data, dataType){
        for(let i=0; i< data.msg.length; i++){
            var m = data.msg[i];
            if(m.id == msg_id){
                if(i + delta < 0){
                    alert("MSG" + msg_id + "は先頭のメッセージです");
                    return;
                }
                if(i + delta >= data.msg.length){
                    alert("MSG" + msg_id + "は最後のメッセージです");
                    return;
                }
                var next_id = data.msg[i+delta].id;
                location.href = "/edit?id=" + next_id;
                return;
            }
        }
        alert("MSG" + msg_id + "が見つかりません");
    }});
}


function titleSet()
{
    var player = $("#overlay-select").val();
    var text = "";
    
    if(player == "static-text" || player == "scroll-text"){
        text = $("#" + player + "-text").val();
        $("#msg-title").val(text.substring(0,16));
    }
}

function new_msg()
{
    if(modified()){
        if(!confirm("データが変更されています。\n" +
                    "書込後、新規メッセージを作成しますか？"))
            return;
        writeMsg();
    }
    
    load_index({success:function(data, dataType){
        var id = 0;
        for(let m of data.msg)
            id = Math.max(id, m.id);
        id++;
        make_new_msg(data, id);
        location.href = "/edit?id=" + id; 
    }});
}

function bitmap_file()	// 画像ファイル・変更時
{
    console.log("bitmap_file");
    var f = $("#btn-bitmap-file")[0];
    if(f.files.length > 0){
        var file = f.files[0];
        var img = $("#msg-bitmap")[0];
        img.src = URL.createObjectURL(file);
        
        // var img = new Image();
        // img.src = URL.createObjectURL(file);
        // img.onload = function(){
        //     var w = img.width;
        //     var h = img.height;
        //     var s = "<canvas id='canvas'";
        //     s += " width='" + w  + "' height='" + h;
        //     s += "'>";
        //     $("#canvas-container").html(s);
        //     var canvas = $("#canvas")[0];
        //     var ctx = canvas.getContext('2d');
        //     ctx.drawImage(img,0,0);
        // }
    }
}

function upload_bmp()
{
    var canvas = $("#canvas")[0];
    if(canvas){
        var w = canvas.width;
        var h = canvas.height;
        var ctx = canvas.getContext('2d');
        var im = ctx.getImageData(0,0,w,h);
        var buf = new ArrayBuffer(im.data.length + 14 + 40);

        var dv = new DataView(buf);
        dv.setUint8(  0, "B".charCodeAt(0));
        dv.setUint8(  1, "M".charCodeAt(0));
        dv.setUint32( 2, buf.byteLength, true);
        dv.setUint16( 6, 0, true);
        dv.setUint16( 8, 0, true);
        dv.setUint32(10, 14 + 40, true);

        dv.setUint32(14, 40, true);
        dv.setUint32(18,  w, true);
        dv.setUint32(22, -h, true);
        dv.setUint16(26,  1, true);
        dv.setUint16(28, 24, true);
        dv.setUint32(30,  0, true);
        dv.setUint32(34,  im.data.length, true);
        dv.setUint32(38,  255, true);
        dv.setUint32(42,  255, true);
        dv.setUint32(46,  0, true);
        dv.setUint32(50,  0, true);

        var wp = 54
        for(let i=0; i<im.data.length; i+=4){
            dv.setUint8(wp++, im.data[i+2]);
            dv.setUint8(wp++, im.data[i+1]);
            dv.setUint8(wp++, im.data[i]);
        }
        var blob = new Blob([buf], { type: "image/bmp"}); 
        
        status("upload bmp");
        upload_file(msg_id, "bmp", blob, 
                    function(data,dataType){
                        status("upload bmp succeeded");
                    });
    }
}

function onScrollTypeChange()
{
    var st = $("#bitmap-scrollType").val();
    var readonly = st == 0;
    for(let n of ['scrollout', 'speedX', 'speedY']){
        $("#bitmap-" + n ).prop("readonly", readonly);
    }
}

function msg_bitmap_onload(e){
    var img = e.target;
    $("#msg-bitmap-info").html(
        "画像幅:" + img.width + " 画像高さ:" + img.height
    );
}
