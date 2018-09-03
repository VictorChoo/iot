console.log('Loading function insertToLog');

var AWS = require("aws-sdk");

// AWS.config.update({
//   region: "us-west-2",
//   endpoint: "http://localhost:8000"
// });

var docClient = new AWS.DynamoDB.DocumentClient();

// const
var TableName_MAC = "xxxxxxxx"; // DynamoDB Table name stored MAC address
var TableName_LOG = "xxxxxxxx"; // DynamoDB Table name for storing device log

// response handler
global.statusCode = 200;
global.ErrorStatus = "";
global.responseBody = {};
function responseHandler(statusCode, ErrorStatus) {
    if(statusCode == 200)
    {
        console.log(ErrorStatus);
    }
    else
    {
        console.error(ErrorStatus);
    }
     
    global.responseBody["statusCode"] = statusCode;
    global.responseBody["ErrorStatus"] = ErrorStatus;
    
    // Make response
    global.response = {
        "statusCode": global.statusCode,
        "headers": {
            'Content-Type': 'application/json'
        },
        "body": JSON.stringify(global.responseBody),
        "isBase64Encoded": false
    };
}

exports.handler = (event, context, callback) => {
    // event : input stream
    // context : Unique id in aws
    console.log('Received event:', JSON.stringify(event, null, 2));
    global.statusCode == 200;
    global.ErrorStatus = "";
    global.responseBody = {};
    
    /////////////////////////////////////////////////////////////////////
    // Flow                                                            //
    // 1. Select MAC Address and compare if it is in permission table  //
    // 2. Insert to log table with timestamp(PK)                       //
    /////////////////////////////////////////////////////////////////////

    
    console.log('global.statusCode : ', global.statusCode);
    // If status code is not setting, continue
    // if(global.statusCode == 200)
    // {
        // 1. select MAC Address and compare if it is in permission table 
        // var eventObj = JSON.parse(event.body); // When use API Gateway Any
        var eventObj = event; // When use API Gateway Post
        var selectMacQuery = {
            TableName: TableName_MAC,
            Key:{
                "mac": eventObj.mac
            }
        };
        console.log("selectMacQuery", JSON.stringify(selectMacQuery, null, 2));
        
        docClient.get(selectMacQuery, function(err, data) {
            if (err) {
                global.ErrorStatus = "Error at dynamoDB";
                global.statusCode = 503;
                responseHandler(global.statusCode, global.ErrorStatus);
                callback(null, global.response);
            } else {
                console.log("GetItem succeeded:", JSON.stringify(data, null, 2));
                
                // There are not data on this MAC Address
                if(JSON.stringify(data, null, 2) == "{}")
                {
                    global.ErrorStatus = "****[E] Not Permitted MAC Address: " + eventObj.mac;
                    global.statusCode = 403;
                    responseHandler(global.statusCode, global.ErrorStatus);
                    callback(null, global.response);
                }
                else
                {
        // 3. insert to log table with timestamp(PK)
                    // Make uuid
                    var timestamp = eventObj.timestamp + "_" + eventObj.group + "_" + eventObj.thing_name;
                    // Make Insert Query
                    var insertQuery = {
                        TableName:TableName_LOG,
                        Item:{
                            "timestamp": timestamp,
                            "group": eventObj.group,
                            "thing_name": eventObj.thing_name,
                            "payload": eventObj
                        }
                    };
                    
                    docClient.put(insertQuery, function(err, data) {
                        if (err) {
                            global.ErrorStatus = "Unable to add item. Error JSON:" + JSON.stringify(err, null, 2);
                            global.statusCode = 500;
                            responseHandler(global.statusCode, global.ErrorStatus);
                            callback(null, global.response);
                            
                        } else {
                            console.log("Insert Success!!!");
                            // ErrorStatus = "Insert Success!!!";
                            // global.ErrorStatus = "**Insert Success**";
                            global.statusCode = 200;
                            responseHandler(global.statusCode, global.ErrorStatus);
                            callback(null, global.response);
                            
                        }
                    });
                }
            }
        });
    // }
};
