function write_index(data, reload_index = true){
    var formData = new FormData();
    var blob = new Blob([JSON.stringify(data,null,' ')],
                        {type: 'application/json'});
    formData.append("foo",blob);

    $.ajax({
        type: "post",
        url: "/upload_index",
        data: formData,
        contentType: false,
        processData: false,
        dataType: "text",
        success: function(data, datatype){
            status("update_index succeeded");

            if(reload_index){
                $.ajax({
                    type: "get",
                    url: "/api",
                    data: {cmd: "reload_index"},
                    success: function(data, dataType){
                        status("reload_index succeeded.");
                    },
                    error: function(XMLHttpRequest, textStatus, e){
                        status("reload_index failed:" + textStatus
                               + e.message, true);
                    }
                });
            }
        },
        error: function(XMLHttpRequest, textStatus, e){
            status("update_index failed:" + textStatus + e.message, true);
        }
    })
}

function status (msg, error = false) {
    $("#status").text(msg);
    if(error){
        console.error(msg);
    } else {
        console.log(msg);
    }
}

function read_config(callback = null){
    status("read config");
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "json",
        data: {cmd: "read_config"},
        success: function(data, dataType){
            status("read config succeeded");
            if(callback){
                callback(data, dataType);
            }
        },
        error: function(xhr, textStatus, e){
            status("error:"+textStatus+":"+e.message, true);
        }
    });
}

function upload_file(id, type, obj, callback = null){
    var blob;
    
    if(obj instanceof Blob){
        blob = obj;
    } else if( 'string' == typeof obj ) {
        blob  = new Blob([obj],{type: 'application/octet-binary'});
    } else {
        blob = new Blob([JSON.stringify(obj,null,' ')],
                        {type: 'application/json'});
    }
    var formData = new FormData();
    formData.append("upload",blob,id);

    $.ajax({
        type: "post",
        url: "/upload_" + type,
        dataType: "text",
        data: formData,
        cache: false,
        contentType: false,
        processData: false,
        success: function(data, dataType){
            status("upload_file succeeded.");
            if(callback){
                callback(data, dataType);
            }
        },
        error: function(xhr, text, e){
            status("upload_file failed:" + text + ":" +
                   xhr.responseText, true);
        },
    });
}

function upload_file2(filename, obj, opt){
    var blob;
    
    if(obj instanceof Blob){
        blob = obj;
    } else if( 'string' == typeof obj ) {
        blob  = new Blob([obj],{type: 'application/octet-binary'});
    } else {
        blob = new Blob([JSON.stringify(obj,null,' ')],
                        {type: 'application/json'});
    }
    var formData = new FormData();
    formData.append("upload", blob, filename);
    
    var ajaxObj = {
        type: "post",
        url: "/upload_file",
        dataType: "text",
        data: formData,
        cache: false,
        contentType: false,
        processData: false,
        error: function(xhr, t, e){
            status("upload_file2 failed:" + t + ":" + e.message, true);
        }
    };

    for(let k in opt){
        ajaxObj[k] = opt[k];
    }

    $.ajax(ajaxObj);
}


function device_control(cmd)
{
    var data;
    if(cmd == 'clear'){
        data = { cmd: 'stop', clear: 1 };
    } else if(cmd == 'stop'){
        data = { cmd: 'stop' };
    } else if(cmd == 'start'){
        data = { cmd: 'start'};
    } else if(cmd == 'reload'){
        data = { cmd: 'start', clear:1, reload: 1};
    } else {
        status("cmd:" + cmd + " not defined.", true);
        return;
    }
    
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "text",
        data: data,
        success: function(data, dataType){
            status("device_control(" + cmd + ") succeeded.");
        },
        error: function(xhr, text, e){
            status:("device_control(" + cmd + ") :" + text +
                    xhr.responseText, true);
        }
    });
}

function disp_msg(id)
{
    $.ajax({
        type: "get",
        url: "/api",
        dataType: "text",
        data: { cmd: "disp_msg", id: id },
        success: function(data, dataType){
            status("disp_msg(" + id + ") succeeded.");
        },
        error: function(xhr, text, e){
            status:("device_control(" + cmd + ") :" + text +
                    xhr.responseText, true);
        }
    });
}

function load_index(opt) {
    var ajaxObj = {
        type: "get",
        url: "/api",
        dataType: "json",
        data: { cmd : "read_index" },
        error: function(xhr, status, error){
            console.log(status + ':' + xhr.responseText);
        }
    };

    for(let prop in opt){
        if(prop == "script")
            ajaxObj.data.script = opt.script;
        else if(prop == "dataType")
            ajaxObj.dataType = opt.dataType;
        else if(prop == "success" || prop == "error")
            ajaxObj[prop] = opt[prop];
        else
            throw new Error("Bad option:" + prop); 
    }
    
    $.ajax(ajaxObj);
}


function make_new_msg(index, id)
{    
    index.msg.push( {
        id: id,
        enable: true,
        title: "メッセージ" + id,
        player: "static-text",
    });
    write_index(index);
    upload_file(id, 'msg', {
        player: "static-text",
        title:  "メッセージ" + id,
        "pre-clear": true,
        "post-clear": true,
        time: 1.0,
        text: "MSG" + id,
    });
}

function read_msg_file(id, opt)
{
    var ajaxObj = {
        type: "get",
        url: "/api",
        dataType: "json",
        data: {cmd : "read_msg", id:id },
        success : function(data, dataType){
            console.log("read_msg_file succeeded");
        },
        error: function(xhr, text, e){
            console.log("read_msg_file failedr:" + text + ":"
                        + xhr.responseText);
        }
    };

    for(let key in opt){
        if(key == "fileType")
            ajaxObj.data.cmd = "read_" + opt.fileType;
        else if(key == "script")
            ajaxObj.data.script = opt.script;
        else
            ajaxObj[key] = opt[key];
    }
    $.ajax(ajaxObj);
}

function msg_name (id) {
    return "msg" + ("00" + id).slice(-2);
}

function new_script(new_name, opt = {}){
    var ajaxObj = {
        url: "/api",
        type: "GET",
        data: {cmd : "new_script", script: new_name},
        dataType: "json",
        error: function(xhr, text, e){
            console.log(text + ":" + xhr.responseText);
        }
    };
    for(let key in opt){
        ajaxObj[key] = opt[key];
    }
    $.ajax(ajaxObj);
}

