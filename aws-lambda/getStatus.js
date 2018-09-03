console.log('Loading function getStatus');

var AWS = require("aws-sdk");

// AWS.config.update({
//   region: "us-west-2",
//   endpoint: "http://localhost:8000"
// });

var docClient = new AWS.DynamoDB.DocumentClient();

// const
var TableName_Status = "xxxxxxxx"; // DynamoDB Table name stored device status


// response handler
global.statusCode = 200;
global.responseStatus = {};
global.responseBody = {};
function responseHandler(statusCode, responseStatus) {
    console.log("statusCode : ", statusCode);
    console.log("responseStatus : ", responseStatus);
    
    global.responseBody["StatusCode"] = statusCode;
    global.responseBody["ResponseStatus"] = responseStatus;
    
    // Make response
    global.response = {
        "statusCode": global.statusCode,
        "headers": {
            'Content-Type': 'application/json',
        },
        "body": JSON.stringify(global.responseBody),
        "isBase64Encoded": false
    };
}


exports.handler = (event, context, callback) => {
    // event : input stream
    // context : Unique id in aws
    console.log('Received event:', JSON.stringify(event, null, 2));
    console.log('Received context:', JSON.stringify(context, null, 2));
    
    global.statusCode == 200;
    global.responseStatus = {};
    global.responseBody = {};
    
    /////////////////////////////////////////////////////////////////////
    // Flow
    // 1. Diverse according to group
    //    1.1. All
    //        - Select all status
    //    1.2. else group
    //        - Select status about this group
    //      1.2.1. Diverse according to type
    //          1.2.1.1. nodemcu
    //              - Make unique status in input_group
    //          1.2.1.2. else thing
    //              - response json data including {group, thing, status} 
    /////////////////////////////////////////////////////////////////////
    
    console.log('global.statusCode : ', global.statusCode);
    // If status code is not setting, continue
    // if(global.statusCode == 200)
    // {
        // 1. Diverse according to group
        // var eventObj = JSON.parse(event.body); // When use API Gateway Any
        var tmp_count = "count";
        var tmp_body = "json";
        var tmp_total_status = {}; // empty Object to response
        tmp_total_status[tmp_body] = []; // empty Array, which you can push() values into
        
        // var eventObj = event; // When use API Gateway Post
        var eventObj = event.queryStringParameters; // When use API Gateway Post(Lambda proxy)
        var input_group = eventObj.group;
        var input_type = eventObj.type;
        
        console.log("eventObj : ", eventObj);
        console.log("input_group : ", input_group);
        console.log("input_type  : ", input_type);
        
        // 1.1. All - Select all status
        if(input_group == "ALL")
        {
            console.log("input_group is All!!");
            
            var scanAllQuery = {
                TableName: TableName_Status
            };
            
            console.log("Select All Query : ", scanAllQuery);
            
            docClient.scan(scanAllQuery, function(err, data){
                if (err) {
                    console.log(err);
                    global.responseStatus = "Error at dynamoDB";
                    global.statusCode = 503;
                    responseHandler(global.statusCode, global.responseStatus);
                    callback(null, global.response);
                } else {
                    console.log("Get All Item succeeded:", JSON.stringify(data, null, 2));
                    // There are not data on table
                    if(JSON.stringify(data, null, 2) == "{}")
                    {
                        global.ErrorStatus = "****[E]Not Found. group name : " + input_group;
                        global.statusCode = 403;
                        responseHandler(global.statusCode, global.ErrorStatus);
                        callback(null, global.response);
                    }
                    else
                    {
                        var i = 0;
                        
                        data.Items.forEach(function(itemdata) {
                            i = i + 1;
                            console.log("Item :", i,"th : ", JSON.stringify(itemdata));
                            
                            var tmp_each_status = {};
                            
                            // Make each json data
                            tmp_each_status["group"] = itemdata.group;
                            tmp_each_status["thing_name"] = itemdata.thing_name;
                            tmp_each_status["status"] = itemdata.status;
                            
                            // Set total json data
                            tmp_total_status[tmp_body].push(tmp_each_status);
                            
                        });
                        
                        console.log("count : ", i);
                        // console.log("body : ", JSON.stringify(tmp_total_status));
                        
                        // Set response data
                        tmp_total_status[tmp_count] = i;
                        console.log("body : ", JSON.stringify(tmp_total_status));
                        
                        global.responseStatus = tmp_total_status;
                        
                        global.statusCode = 200;
                        responseHandler(global.statusCode, global.responseStatus);
                        callback(null, global.response);
                    }   
                }
            });
        }
        // 1.2. else group - Select status about this group
        else
        {
            console.log("input_group is : ", input_group);
            
            // Make query using on partial match search
            var scanPartialSearchQuery = {
                    TableName: TableName_Status,
                    FilterExpression: 'begins_with(group_thing, :t) ',
                    ExpressionAttributeValues: {
                        ":t": input_group
                    }
            };
            
            console.log("Partial select Query : ", scanPartialSearchQuery);
            
            // Search partial match
            docClient.scan(scanPartialSearchQuery, function(err, data){
                if (err) {
                    console.log(err);
                    global.responseStatus = "Error at dynamoDB";
                    global.statusCode = 503;
                    responseHandler(global.statusCode, global.responseStatus);
                    callback(null, global.response);
                } else {
                    console.log("Get Partial Item succeeded:", JSON.stringify(data, null, 2));
                    
                    if(JSON.stringify(data, null, 2) == "{}")
                    {
                        global.ErrorStatus = "****[E]Not Found. group name : " + input_group;
                        global.statusCode = 403;
                        responseHandler(global.statusCode, global.ErrorStatus);
                        callback(null, global.response);
                    }
                    else
                    {
                        var i = 0;
                        // 1.2.1. Diverse according to type
                        // 1.2.1.1. nodemcu - Make unique status in input_group
                        if(input_type == "nodemcu")
                        {
                            console.log("nodemcu start!! ");
                            
                            var tmp_status = 1;
                            data.Items.forEach(function(itemdata) {
                                i = i + 1;
                                console.log("Item :", i,"th : ", JSON.stringify(itemdata));
                                // if status is 0 atleast 1 thing, status is 0
                                if(itemdata.status == 0)
                                {
                                    tmp_status = 0;
                                }
                            });
                            
                            // Set response data
                            tmp_total_status["status"] = tmp_status;
                            console.log("body : ", JSON.stringify(tmp_total_status));
                            
                            global.responseStatus = tmp_total_status;
                            global.statusCode = 200;
                            responseHandler(global.statusCode, global.responseStatus);
                            callback(null, global.response);
                        }
                        // 1.2.1.2. else thing - response json data including {group, thing, status} 
                        else if(input_type == "Android")
                        {
                            console.log("Android start!!");
                            
                            data.Items.forEach(function(itemdata) {
                                i = i + 1;
                                console.log("Item :", i,"th : ", JSON.stringify(itemdata));
                                
                                var tmp_each_status = {};
                                
                                // Make each json data
                                tmp_each_status["group"] = itemdata.group;
                                tmp_each_status["thing_name"] = itemdata.thing_name;
                                tmp_each_status["status"] = itemdata.status;
                                
                                // Set total json data
                                tmp_total_status[tmp_body].push(tmp_each_status);
                                
                            });
                            
                            console.log("count : ", i);
                            // console.log("body : ", JSON.stringify(tmp_total_status));
                            
                            // Set response data
                            tmp_total_status[tmp_count] = i;
                            console.log("body : ", JSON.stringify(tmp_total_status));
                            
                            global.responseStatus = tmp_total_status;
                            global.statusCode = 200;
                            responseHandler(global.statusCode, global.responseStatus);
                            callback(null, global.response);
                        }
                    }
                }
            });
        }
    // }
};
