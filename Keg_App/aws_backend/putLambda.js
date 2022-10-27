const AWS = require('aws-sdk');
AWS.config.update({region: 'us-east-2'});

exports.handler = async (event, context) => {
    
    console.log("Input from MCU: " + JSON.stringify(event));
    
    // if no kegId is provided, exit
    if (event.bodyjson.kegId == null) return {"response":"no kegid stated in request"};
    
    // create a Document Client object 
    const dc = new AWS.DynamoDB.DocumentClient();
    
    // store data from event.bodyjson into newdata variable 
    var newdata = event.bodyjson.data;
    
    // add a time entry (the identifier) to the newdata variable
    newdata.time = Date.now().toString();
    
    // Create a new keg partition, unless it already exists
    const newitem_params = {
        TableName: 'kegs',
        Item: {
            "kegId": event.bodyjson.kegId,
            "data": [newdata]
        },
        ConditionExpression: 'attribute_not_exists(kegId)'
    };
    
    console.log("NewItemParams: " + JSON.stringify(newitem_params));
    
    // Put the new keg partition into the Document Client
    // if the keg partition exists, use nested function to update the partition instead
    
    var result = "nothing happened";
    
    try {
        result = await dc.put(newitem_params).promise();
        console.log("PutItem Succeeded:", JSON.stringify(result,null,2));
    } catch(err) {
        console.log("Keg partition already exists. Using UPDATE instead.");
            
        // append new entry to exisisting data set
        const append_params = {
            TableName: "kegs",
            Key: {
                "kegId": event.bodyjson.kegId
            },
            UpdateExpression: 'set #data = list_append(#data, :entry)',
            ExpressionAttributeNames: {
                '#data' : 'data'
            },
            ExpressionAttributeValues: {
                ':entry': [newdata]
            }
        }; 
        
        console.log("AppendParams: " + JSON.stringify(append_params));
        
        try {
            result = await dc.update(append_params).promise();
            console.log("UpdateItem Succeeded: ", JSON.stringify(result, null, 2));
        } catch(err) {
            console.log("UpdateItem !!! FAILED !!!. Error JSON: ", JSON.stringify(err, null, 2));
        }

    }
    return result;
}
