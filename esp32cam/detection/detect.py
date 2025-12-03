import cv2
import numpy as np
import urllib.request
import threading

# ==========================================
# CONFIGURATION
# ==========================================
url = 'http://10.64.135.54:81/stream' 
confThreshold = 0.5
nmsThreshold = 0.4

# ==========================================
# 1. THREADED CAMERA CLASS (ELIMINATES LAG)
# ==========================================
class VideoStream:
    def __init__(self, src=0):
        self.stream = cv2.VideoCapture(src)
        (self.grabbed, self.frame) = self.stream.read()
        self.stopped = False

    def start(self):
        # Start the thread that reads frames from the video stream
        threading.Thread(target=self.update, args=()).start()
        return self

    def update(self):
        # Keep looping indefinitely until the thread is stopped
        while True:
            if self.stopped:
                return
            # Grab the latest frame (discarding the buffer implicitly by overwriting self.frame)
            (self.grabbed, self.frame) = self.stream.read()

    def read(self):
        # Return the most recent frame
        return self.frame

    def stop(self):
        self.stopped = True

# ==========================================
# SETUP YOLO MODEL
# ==========================================
classesFile = "coco.names"
classes = None
with open(classesFile, 'rt') as f:
    classes = f.read().rstrip('\n').split('\n')

colors = np.random.uniform(0, 255, size=(len(classes), 3))
modelConfiguration = "yolov3.cfg"
modelWeights = "yolov3.weights"

net = cv2.dnn.readNetFromDarknet(modelConfiguration, modelWeights)
net.setPreferableBackend(cv2.dnn.DNN_BACKEND_OPENCV)
net.setPreferableTarget(cv2.dnn.DNN_TARGET_CPU)

def findObjects(outputs, img):
    hT, wT, cT = img.shape
    bbox = []
    classIds = []
    confs = []

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
            box = bbox[i]
            x, y, w, h = box[0], box[1], box[2], box[3]
            color = colors[classIds[i]]
            cv2.rectangle(img, (x, y), (x+w, y+h), color, 2)
            cv2.putText(img, f'{classes[classIds[i]].upper()} {int(confs[i]*100)}%', 
                        (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)

# ==========================================
# MAIN LOOP
# ==========================================
print(f"Connecting to {url} ...")

# START THE THREADED STREAM
cap = VideoStream(url).start()

while True:
    img = cap.read() # Always gets the LATEST frame
    
    # Safety check in case camera isn't ready
    if img is None:
        continue

    # Prepare image for YOLO
    blob = cv2.dnn.blobFromImage(img, 1/255, (320, 320), [0,0,0], 1, crop=False)
    net.setInput(blob)

    layerNames = net.getLayerNames()
    outputNames = [layerNames[i - 1] for i in net.getUnconnectedOutLayers()]
    outputs = net.forward(outputNames)

    findObjects(outputs, img)

    cv2.imshow('ESP32 Object Detection (Low Latency)', img)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        cap.stop()
        break

cv2.destroyAllWindows()