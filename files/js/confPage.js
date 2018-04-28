var config_data;

$(function(){
    document.title = "設定 / " + document.title;
    
    $("#btn-write").on('click',config_write);
    $("#menu-script-new").on('click',function(){
        $("#new-script-dialog").dialog("open");
    });
    $("#menu-script-delete").on('click',delete_script);
    $("#menu-script-copy").on('click',function(){
        $("#copy-script-src").html( $("#config-script").html());
        $("#copy-script-dialog").dialog("open");
    });
    $("#menu-script-download").on('click',download_script);
    $("#menu-script-upload").on('click',function(){
        $("#script-upload-dialog").dialog("open");
    });
    
    $(window).on('beforeunload', function(e){
        if(modified()){
            return "ちょっと待ってください。";
        }
    });

    $("#new-script-dialog").dialog({
        autoOpen: false,
        height: 150,
        width:  400,
        modal: false,
        buttons: {
            "新規作成" : function(){
                var name = $("#config-script-name").val();
                if(name == ""){
                    alert("スクリプト名を入力して下さい");
                } else if(!/^\w{1,8}$/.test(name)){
                    alert("スクリプト名は英数字8文字以下にして下さい");
                } else {
                    new_script(name, {
                        success: function(data, datatype){
                            set_script_list(data);
                        },
                        error: function(xhr, textStatus, e){
                            console.log(textStatus + ":" +
                                        xhr.responseText);
                            alert("スクリプトの作成に失敗しました。"
                                  + xhr.responseText);
                        }
                    });

                    $(this).dialog("close");
                }
            },
            "キャンセル" : function(){
                $(this).dialog("close");
            }
        },
    });

    $("#copy-script-dialog").dialog({
        autoOpen: false,
        height: 300,
        width:  300,
        modal: false,
        buttons: {
            "コピー" : function(){
                if(copy_script())
                    $(this).dialog("close");
            },
            "名前変更" : function(){
                if(copy_script(true))
                    $(this).dialog("close");
            },
            "キャンセル" : function(){
                $(this).dialog("close");
            }
        },
    });

    $("#script-upload-dialog").dialog({
        autoOpen: false,
        height: 200,
        width:  400,
        modal: false,
        buttons: {
            "アップロード" : function(){
                upload_script();
                $(this).dialog("close");
            },
            "キャンセル" : function(){
                $(this).dialog("close");
            }
        },
    });

    read_config(function(data,dataType){
        config_data = data;
        for(let prop in data){
            $("#config-" + prop).val(data[prop]);
        }
        read_script_list(data.script);
    });
});

function read_script_list( script_name = null)
{
    $.ajax({
        url: "/api",
        type : "GET",
        data: { cmd : "list_script" },
        dataType: "json",
        success: function(data, dataType){
            set_script_list(data, script_name);
        },
        error: function(XMLHttpRequest, textStatus, e){
            console.log("error:" + textStatus + ":" + e.message);
        },
    });
}

function set_script_list(list, script_name = null)
{
    var s = "";

    if(!script_name) 
        script_name = $("#config-script").val();
    list.sort();
    for(let n of list){
        s += "<option value='" + n + "'";
        if(n == script_name)
            s += " selected";
        s += ">";
        s += n;
        s += "</option>";
    }
    $("#config-script").html(s);
}

function get_data(){
    var  d = {};
    for(let name of ['moduleType','moduleRow','moduleColumn',
                     'plane','bright','script','ledMode']){
        d[name] = $("#config-" + name).val();
    }
    return d;
}

function config_write()
{
    status('設定書込');

    var d = get_data();
    var formData = new FormData();
    var blob = new Blob([JSON.stringify(d,null,' ')],{
        type:'application/json'});
    formData.append("foo",blob);

    config_data = d;

    $.ajax({
        url: "/upload_config",
        type: "POST",
        data: formData,
        cache: false,
        contentType : false,
        processData : false,
        dataType: "html"
    }).done(function(data, textStatus,jqXHR){
        status('設定書込完了');
        $.ajax({
            url: "/api",
            type: "POST",
            data: {cmd : "reload_config"},
        }).done(function(data,textStatus,jqXHR){
            status('設定再読込完了');
        }).fail(function(jqXHR,textStatus,errorThrown){
            status('設定再読込失敗');
        });
    })
    .fail(function(jqXHR, textStatus, errorThrown){
        status('設定書込失敗');
    });
}

function modified(){
    var d = get_data();
    var dc = 0;
    for(let n in config_data){
        if(config_data[n] != d[n]){
            console.log(n + " differ " + config_data[n] + " : " + d[n]);
            dc++;
        }
    }

    return dc > 0;
}

function download_script(){
    var script = $("#config-script").val();
    if(!confirm("スクリプト"+script +"をzipファイルでダウンロードしますか?"))
        return;
    
    var zip = new JSZip();
    var folder = zip.folder(script);
    var obj = {
        download : function(){     // index.jsnをダウンロード
            status("load " + script + " index");
            load_index({
                script: script,
                success: function(data, dataType){
                    folder.file("index.jsn", JSON.stringify(data,null,' '));
                    obj.msgs = data.msg;
                    obj.download_msgs();
                },
                error: function(xhr,text,e){
                    status("load " + script + " index failed"
                           + "\n" + e.responseText );
                }
            });
        },
        
        download_msgs : function(){  
            var msg = obj.msgs.shift();
            if(!msg){
                obj.download_done();
            } else {
                var name = msg_name(msg.id);
                status("download " + name);
                read_msg_file(msg.id, {
                    script: script,
                    success: function(data, dataType){
                        folder.file(name + ".jsn",
                                    JSON.stringify(data,null,' '));
                        if(data.player == 'scroll-text'){
                            status("download " + name + ".txt");
                            read_msg_file(msg.id, {
                                script: script,
                                type: "txt",
                                dataType: "text",
                                success: function(data,dataType){
                                    folder.file(name + ".txt",data);
                                    obj.download_msgs();
                                },
                                error: function(xhr,text,e){
                                    status("download " + name + ".txt failed"
                                           + "\n" + e.responseText );
                                }
                            });
                        }
                        else {
                            obj.download_msgs();
                        }
                    },
                    error: function(xhr,text,e){
                        status("download " + name + "failed"
                               + "\n" + e.responseText );
                    }
                });
            }
        },

        download_done : function(){
            status("download done");
            zip.generateAsync({type:"blob"}).then(function(blob){
                saveAs(blob, script + ".zip");
            });
        }
    }

    obj.download();
}


function upload_script(){
    // 内容チェック
    var f = $("#config-upload-file")[0].files[0];
    var zip = new JSZip();
    var script_name1 = f.name.split('.')[0];
    var script_name2;
    var script;
    var files;
    
    console.log("script_name1:" + script_name1);
    
    zip.loadAsync(f)
        .then(function(z){
            read_index();
        }, function(e){
            alert("Zipファイルの読み込みに失敗しました");
            console.log("Error reading " + f.name + ": " + e.message);
        });
    
    // index.jsn読み込み
    function read_index() {
        var files = zip.file(/^\w+\/index.jsn/i);
        if(!files || files.length != 1){
            alert("index.jsn not found\n" +
                  "スクリプトのzipファイルではありません。");
            return;
        }
        script_name2 = files[0].name.split('/')[0];
        script = script_name2;
        make_folder();
    }
    
    // スクリプト・フォルダ作成
    function make_folder(){
        new_script(script, {
            success: function(data, dataType){
                set_script_list(data);
                files = [];
                zip.forEach(function(relativePath, file){
                    files.push({path: relativePath, file: file});
                });
                write_file();
            },
            error: function(xhr, text, e){
                alert("スクリプトの生成に失敗しました。\n" + xhr.responseText);
            }
        });
    }
    // 各ファイル書込

    function write_file(){
        var f = files.shift();
        while(f && f.file.dir)
            f = files.shift();
        if(f){
            f.file.async("string").then(function(str){
                status("upload " + f.path);
                upload_file2( f.path, str, {
                    success: function(data,dataType){
                        write_file();
                    }
                });
            });
        } else {
            status("upload script done");
        }
    }
}

function delete_script() {
    var script = $("#config-script").val();
    if(!confirm("スクリプト " + script + "を削除しますか？"))
        return;
    status("スクリプト" + script + "を削除中");
    $.ajax({
        url: "/api",
        type : "GET",
        data: { cmd : "delete_script", script: script },
        dataType: "json",
        success: function(data, dataType){
            set_script_list(data);
            status("スクリプト" + script +"削除完了")
        },
        error: function(xhr, text, e){
            status("スクリプト" + script +"削除失敗\n"
                   + text + ":" + xhr.responseText);
        },
    });
}

function copy_script(bDelete = false){
    var src = $("#copy-script-src").val();
    var dst = $("#copy-script-dst").val();

    if(dst == ""){
        alert("コピー先スクリプト名を入力して下さい");
        return false;
    } else if(!/^\w{1,8}$/.test(dst)){
        alert("コピー先スクリプト名は英数字8文字以下にして下さい");
        return false;
    } else {
        status(src + "を" + dst + "に" +
               (bDelete ? "移動" : "コピー") + "中");
        $.ajax({
            url: "/api",
            type: "GET",
            data: {cmd : "copy_script", src: src, dst: dst, delete: bDelete},
            dataType: "json",
            success: function(data,dataType){
                set_script_list(data);
                status("スクリプト" + (bDelete ? "移動" : "コピー") + "完了"); 
            },
            error: function(xhr, text, e){
                status("スクリプト・コピーに失敗しました\n"
                       + text + ":" + xhr.responseText, true);
                alert("スクリプト・コピーに失敗しました\n"
                      + text + ":" + xhr.responseText);
            }
        });
        return true;
    }
}
