import cv2
import numpy as np
import paho.mqtt.client as mqttclient
import time
import json
import threading
import urllib.request

# ==========================================
# 1. CORE IOT / MQTT CONFIGURATION 
# ==========================================
BROKER_ADDRESS = "app.coreiot.io"
PORT = 1883
ACCESS_TOKEN = "xgGRJfPwCigNXnIfdUIS" # Your token
ACCESS_USERNAME = "car_detector"

# ==========================================
# 2. CAMERA & YOLO CONFIGURATION
# ==========================================
url = 'http://10.210.234.54:81/stream' 
confThreshold = 0.5
nmsThreshold = 0.4
SEND_INTERVAL = 2  # Seconds between MQTT messages (to prevent flooding)

# ==========================================
# 3. MQTT CALLBACK FUNCTIONS
# ==========================================
def connected(client, usedata, flags, rc):
    if rc == 0:
        print("Connected to CoreIOT successfully!!")
    else:
        print(f"Connection failed with code {rc}")

client = mqttclient.Client("IOT_DEVICE_2")
client.username_pw_set(ACCESS_TOKEN)
client.on_connect = connected
client.connect(BROKER_ADDRESS, 1883)
client.loop_start() # Run MQTT in a background thread

# ==========================================
# 4. CAMERA CLASS (Optimized)
# ==========================================
class VideoStream:
    def __init__(self, src=0):
        self.stream = cv2.VideoCapture(src)
        (self.grabbed, self.frame) = self.stream.read()
        self.stopped = False

    def start(self):
        threading.Thread(target=self.update, args=()).start()
        return self

    def update(self):
        while True:
            if self.stopped:
                return
            (self.grabbed, self.frame) = self.stream.read()

    def read(self):
        return self.frame

    def stop(self):
        self.stopped = True

# ==========================================
# 5. SETUP YOLO
# ==========================================
classesFile = "coco.names"
classes = None
with open(classesFile, 'rt') as f:
    classes = f.read().rstrip('\n').split('\n')

# Note: Ensure these files are in the same folder
modelConfiguration = "yolov3.cfg"
modelWeights = "yolov3.weights" 

net = cv2.dnn.readNetFromDarknet(modelConfiguration, modelWeights)
net.setPreferableBackend(cv2.dnn.DNN_BACKEND_OPENCV)
net.setPreferableTarget(cv2.dnn.DNN_TARGET_CPU)

# ==========================================
# 6. MAIN LOGIC LOOP
# ==========================================
print(f"Connecting to Camera at {url} ...")
cap = VideoStream(url).start()

last_send_time = 0

while True:
    img = cap.read()
    
    if img is None:
        continue

    # Prepare image for YOLO
    blob = cv2.dnn.blobFromImage(img, 1/255, (320, 320), [0,0,0], 1, crop=False)
    net.setInput(blob)
    layerNames = net.getLayerNames()
    outputNames = [layerNames[i - 1] for i in net.getUnconnectedOutLayers()]
    outputs = net.forward(outputNames)

    # --- PROCESS DETECTION ---
    hT, wT, cT = img.shape
    bbox = []
    classIds = []
    confs = []
    
    car_detected_this_frame = False # Reset flag for this frame

    for output in outputs:
        for det in output:
            scores = det[5:]
            classId = np.argmax(scores)
            confidence = scores[classId]
            if confidence > confThreshold:
                w, h = int(det[2]*wT), int(det[3]*hT)
                x, y = int((det[0]*wT) - w/2), int((det[1]*hT) - h/2)
                bbox.append([x, y, w, h])
                classIds.append(classId)
                confs.append(float(confidence))

    indices = cv2.dnn.NMSBoxes(bbox, confs, confThreshold, nmsThreshold)

    if len(indices) > 0:
        for i in indices.flatten():
            # Get class name
            current_class = classes[classIds[i]] #
            
            # CHECK IF IT IS A CAR
            if current_class == 'car':
                car_detected_this_frame = True

            # Draw box (Optional - for UI)
            box = bbox[i]
            x, y, w, h = box[0], box[1], box[2], box[3]
            cv2.rectangle(img, (x, y), (x+w, y+h), (0, 255, 0), 2)
            cv2.putText(img, f'{current_class.upper()} {int(confs[i]*100)}%', 
                        (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

    # --- PUSH DATA TO COREIOT ---
    current_time = time.time()
    
    # Send data only if interval passed to avoid flooding
    if (current_time - last_send_time) > SEND_INTERVAL:
        
        # Prepare telemetry data
        payload = {
            'car_detected': car_detected_this_frame,
            'status': "Car Found" if car_detected_this_frame else "Empty"
        }
        
        print(f"Publishing to CoreIOT: {payload}")
        client.publish('v1/devices/me/telemetry', json.dumps(payload), 1)
        
        last_send_time = current_time

    # Show video
    cv2.imshow('CoreIOT Car Detection', img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        cap.stop()
        break

cv2.destroyAllWindows()
client.loop_stop()