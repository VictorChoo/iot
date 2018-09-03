from __future__ import print_function # Python 2/3 compatibility
import boto3
import json
import decimal
import sys
import logging
from boto3.dynamodb.conditions import Key, Attr
from botocore.exceptions import ClientError

#Setting log
logger = logging.getLogger()
logger.setLevel(logging.INFO)

#Define barrier
PIR_YN = 0          # if variable value is bigger then barrier, human is existing
DISTANCE_YN = 202   # if variable value is smaller then barrier, human is existing

# Helper class to convert a DynamoDB item to JSON.
class DecimalEncoder(json.JSONEncoder):
    def default(self, o):
        if isinstance(o, decimal.Decimal):
            if o % 1 > 0:
                return float(o)
            else:
                return int(o)
        return super(DecimalEncoder, self).default(o)

# dynamodb = boto3.resource('dynamodb', endpoint_url="dynamodb.ap-northeast-2.amazonaws.com") 
dynamodb = boto3.resource("dynamodb", region_name='ap-northeast-2')

def handler(event, context): #This is like main()
    logger.info("handler start!!")
    status = 0
    result = 0
    logger.info(str(event))
    logger.info(str(context))
    
    # Running per receive data
    for record in event['Records']:
        if record['eventName'] == 'INSERT':
            logger.info(record['dynamodb']['NewImage']['payload']['M'])
            
            group      = record['dynamodb']['NewImage']['payload']['M']['group']['S']
            thing_name = record['dynamodb']['NewImage']['payload']['M']['thing_name']['S']
            pir        = int(record['dynamodb']['NewImage']['payload']['M']['pir']['N'])
            ultra      = int(record['dynamodb']['NewImage']['payload']['M']['ultra']['N'])
            
            group_thing= group + '_' + thing_name
            
            logger.info("group : %s", group)
            logger.info("thing_name : %s", thing_name)
            logger.info("pir : %s", pir)
            logger.info("ultra : %s", ultra)
            logger.info("group_thing : %s", group_thing)
            
            
            table = dynamodb.Table('xxxxxxxx') // DynamoDB table name
            
            try:
                response = table.get_item(
                    Key={
                        'group_thing' : group_thing
                    }
                )
            except ClientError as e:
                print(e.response)
                print(e.response['Error']['Message'])
                
            else:
                print(response)
                item_length = int(response['ResponseMetadata']['HTTPHeaders']['content-length'])
                # Null data have 2 length 
                if item_length < 3:
                    InsertGroup = group
                    InsertThingName = thing_name
                    InsertGroupThingName = InsertGroup + "_" + InsertThingName
                    
                    # 1. pir sensor value is bigger then PIR_YN -> human exist
                    # 2. pir sensor value is smaller then PIR_YN
                    #   2.1. distance value is smaller then DISTANCE_YN -> human exist
                    #   2.2. distance value is bigger  then DISTANCE_YN -> human not exist
                    if pir > PIR_YN:
                        InsertStatus = 1
                    else:
                        if distance < DISTANCE_YN:
                            InsertStatus = 1
                        else:
                            InsertStatus = 0
                            
                    # Insert data 
                    InsertResponse = table.put_item(
                       Item={
                            'group_thing' : InsertGroupThingName,
                            'group': InsertGroup,
                            'thing_name': InsertThingName,
                            'status' : InsertStatus
                        }
                    )

                    print("PutItem succeeded:")
                    print(json.dumps(InsertResponse, indent=4, cls=DecimalEncoder))
                
                else:
                    item = response['Item']
                    print("GetItem succeeded:")
                    # print(json.dumps(item, indent=4, cls=DecimalEncoder))
                    print(item)
                    currentStatus = response['Item']['status']
                    # print(response['Item']['status'])
                    print(currentStatus)
                    
                    # item.group
                    # item.thing_name
                    # item.group_thing
                    # item.status
                    
                    # status is exist(1) -> pir == PIR_YN or distance < DISTANCE_YN -> 1 else 0
                    #       not exist(0) -> pir == PIR_YN   
                    if currentStatus == 1:
                        if pir > PIR_YN or distance < DISTANCE_YN:
                            updateStatus = 1
                        else:
                            updateStatus = 0
                    else:
                        if pir > PIR_YN:
                            updateStatus = 1
                        else:
                            updateStatus = 0
                    
                    response = table.update_item(
                        Key={
                            'group_thing' : group_thing
                        },
                        UpdateExpression="set #st = :val1",
                        ExpressionAttributeValues={
                            ':val1': updateStatus
                        },
                        ExpressionAttributeNames={
                            "#st": "status"
                        }
                        ,
                        ReturnValues="UPDATED_NEW"
                    )
                    
                    print("UpdateItem succeeded:")
                    print(json.dumps(response, indent=4, cls=DecimalEncoder))


