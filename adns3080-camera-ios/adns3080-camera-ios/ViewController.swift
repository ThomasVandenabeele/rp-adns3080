//
//  ViewController.swift
//  adns3080-camera-ios
//
//  Created by Vandenabeele Thomas on 24/11/2016.
//  Copyright © 2016 Vandenabeele Thomas. All rights reserved.
//


import UIKit

class ViewController: UIViewController {

    @IBOutlet weak var xOffsetLabel: UILabel!
    @IBOutlet weak var yOffsetLabel: UILabel!
    var inputTextField: UITextField?
    let backgroundGrad = CAGradientLayer()
    @IBOutlet weak var frame: UIImageView!
    
    let f = Test_View(frame: CGRect(x:40, y:450, width: 300, height: 180))
   
    
    var socket: SocketIOClient?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        
        //backgroundGrad.frame = self.view.bounds
        
        //let colors = [UIColor(red: 0.5, green: 0.5, blue: 0.5, alpha: 1.0).cgColor,
          //            UIColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0).cgColor]
        
        //backgroundGrad.colors = UIColor.lightGray.cgColor
        //view.layer.insertSublayer(backgroundGrad, at: 0)
        
        
        self.view.backgroundColor = UIColor.lightGray
        
        f.backgroundColor = UIColor(white: 1.0, alpha: 0.0)
        view.addSubview(f)
        
    }
    
    override func viewDidAppear(_ animated: Bool) {
        
        // Check it the user in on a simulator is so default to localhost if not prompt for the IPAddress of the example server.
        
        #if (arch(i386) || arch(x86_64))
            socket = SocketIOClient(socketURL: NSURL(string:"http://localhost:8900") as! URL)
            addHandlers()
            socket!.connect()
        #else
            promptUserOnDevice()
        #endif
        
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    
    func addHandlers() {
        //print(socket)
        
        socket?.on("frameData") {
            [weak self] data, ack in
            
            //print("got frame")
            var incoming: [String: AnyObject] = data[0] as! [String : AnyObject]
            
            //if let xoffset = incoming["xoffset"] as? NSNumber{
            
            let xOffset = (incoming["xoffset"])?.doubleValue ?? 0.0
            let yOffset = (incoming["yoffset"])?.doubleValue ?? 0.0
            
            // the number of elements:
            let count = (incoming["pixels"]?.length)! / MemoryLayout<UInt8>.size
            
            // create array of appropriate length:
            var array = [UInt8](repeating: 0, count: count)
            
            // copy bytes into array
            incoming["pixels"]?.getBytes(&array, length:count * MemoryLayout<UInt8>.size)
            
            //print(array)
            let redPixel = PixelData(a: 255, r: 255, g: 0, b: 0)
            var pixelData = [PixelData](repeating: redPixel, count: count)
            
            var index:Int = 0
            for pixel in array {
                let newPixel = PixelData(a: 255, r: pixel, g: pixel, b: pixel)
                pixelData[index] = newPixel
                index = index + 1
            }
            
            // Update pixels
            let image = self?.imageFromARGB32Bitmap(pixels: pixelData, width: 30, height: 30)
            
            self?.frame.image = image
            
            
            //let pixels = (incoming["pixels"] as! NSArray) as! [UInt8]
            //print(pixels.count)
            
            self?.xOffsetLabel.text = String(format:"%.2f", xOffset)
            self?.yOffsetLabel.text = String(format:"%.2f", yOffset)
            
            self?.f.moveOffset(x: Int(xOffset), y: Int(yOffset))
            
            return
        }
        
        //socket?.onAny {print("Got event: \($0.event), with items: \($0.items)")}
    }
    
    // Prompt for user to enter IP Address of the server.
    func promptUserOnDevice() {
        let newWordPrompt = UIAlertController(title: "Server IP Address", message: "Enter the ip address of the server", preferredStyle: UIAlertControllerStyle.alert)
        
        newWordPrompt.addTextField(configurationHandler: {(textField: UITextField) in
            textField.placeholder = "IP Address"
            textField.text = "192.168.0.112"
            self.inputTextField = textField
            
        })
        newWordPrompt.addAction(UIAlertAction(title: "Cancel", style: UIAlertActionStyle.default, handler: nil))
        newWordPrompt.addAction(UIAlertAction(title: "OK", style: UIAlertActionStyle.default, handler:{ (action) -> Void in
            let textfeild = newWordPrompt.textFields![0] as UITextField
            
            guard let ip = textfeild.text else { return }
            print("Attempting to connect to http://" + ip + ":8900")
            self.socket = SocketIOClient(socketURL: NSURL(string: ("http://" + ip + ":8900"))! as URL)
            self.addHandlers()
            self.socket?.connect()
            
        }))
        present(newWordPrompt, animated: true, completion: nil)
        
    }
    
    @IBAction func Clear(_ sender: UIButton) {
        f.clear()
    }

    func renderGrayScott(grayScottData:[GrayScottStruct])->UIImage
    {
        //var startTime = CFAbsoluteTimeGetCurrent();
        
        var pixelArray = [PixelData](repeating: PixelData(a: 255, r:0, g: 0, b: 0), count: 900)
        
        for i in 0 ..< 30
        {
            for j in 0 ..< 30
            {
                let grayScottCell : GrayScottStruct = grayScottData[i * 30 + j]
                let index = i * 30 + j
                let u_I = UInt8(grayScottCell.u * 255)
                pixelArray[index].r = u_I
                pixelArray[index].g = u_I
                pixelArray[index].b = UInt8(grayScottCell.v * 255)
            }
        }
        let outputImage = imageFromARGB32Bitmap(pixels: pixelArray, width: UInt(30), height: UInt(30))
        
        //print(" R RENDER:" + NSString(format: "%.4f", CFAbsoluteTimeGetCurrent() - startTime));
        
        return outputImage
    }
    
    struct GrayScottStruct
    {
        var u : Double = 0.0;
        var v : Double = 0.0;
        
        init(u : Double, v: Double)
        {
            self.u = u;
            self.v = v;
        }
    }
    
    public struct PixelData {
        var a:UInt8 = 255
        var r:UInt8
        var g:UInt8
        var b:UInt8
    }
    
    private let rgbColorSpace = CGColorSpaceCreateDeviceRGB()
    private let bitmapInfo:CGBitmapInfo = CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedFirst.rawValue)
    
    public func imageFromARGB32Bitmap(pixels:[PixelData], width:UInt, height:UInt)->UIImage {
        let bitsPerComponent:UInt = 8
        let bitsPerPixel:UInt = 32
        
        assert(pixels.count == Int(width * height))
        
        var data = pixels // Copy to mutable []
        let providerRef = CGDataProvider(
            data: NSData(bytes: &data, length: data.count * MemoryLayout<PixelData>.size)
        )
        
        let cgim = CGImage(
            width: Int(width),
            height: Int(height),
            bitsPerComponent: Int(bitsPerComponent),
            bitsPerPixel: Int(bitsPerPixel),
            bytesPerRow: Int(width) * Int(UInt(MemoryLayout<PixelData>.size)),
            space: rgbColorSpace,
            bitmapInfo: bitmapInfo,
            provider: providerRef!,
            decode: nil,
            shouldInterpolate: true,
            intent: CGColorRenderingIntent(rawValue: 0)!
        )
        return UIImage(cgImage: cgim!)
    }
}

class Test_View: UIView {
    
    var currentX = 150
    var currentY = 90
    var oldX = 150
    var oldY = 90
    
    var xArray : [Int] = [150]
    var yArray : [Int] = [90]
    
    override init(frame: CGRect) {
        super.init(frame: frame)
    }
    
    required init(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)!
    }
    override func draw(_ rect: CGRect) {
        
        if(!(currentX == oldX && currentY == oldY)){
        
            let path = UIBezierPath()
            path.lineWidth = 3.0
            
            for i in 0..<xArray.count-1{
                let xval:Int = xArray[i]
                let yval:Int = yArray[i]
                
                let xval2:Int = xArray[i+1]
                let yval2:Int = yArray[i+1]
                
                
                path.move(to: CGPoint(x: xval, y: yval))
                path.addLine(to: CGPoint(x: xval2, y: yval2))
             
                UIColor.red.setStroke()
                path.stroke()
            }
            
            
            //var drect = CGRect(x: (w * 0.25),y: (h * 0.25),width: (w * 0.5),height: (h * 0.5))
            //var bpath:UIBezierPath = UIBezierPath(rect: drect)
            
            //color.set()
            //bpath.stroke()
            
            oldX = currentX
            oldY = currentY
            
            //NSLog("drawRect has updated the view")
        
        }
        
    }
    
    func moveOffset(x: Int, y: Int){
        
        if(!(x == 0 && y == 0)){
            //var newX : NSMutableArray = [currentX+x]
            //var newY : NSMutableArray = [currentY-y]
            
            xArray.append(currentX+x*2)
            yArray.append(currentY-y*2)
            
            //xArray.add(newX)
            //yArray.add(newY)
                
            currentX = currentX+x*2
            currentY = currentY-y*2
            
            //print("x: ", newX, ", y: ", newY)
            
            self.setNeedsDisplay()

        }

    }
    
    func clear(){
        xArray = [150]
        yArray = [90]
        
        currentX = 150
        currentY = 90
        oldX = 150
        oldY = 90
        
        setNeedsDisplay()
    }
    
}


